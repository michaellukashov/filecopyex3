/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010
Idea & core: Max Antipin
Coding: Serge Cheperis aka craZZy
Bugfixes: slst, CDK, Ivanych, Alter, Axxie and Nsky
Special thanks to Vitaliy Tsubin
Far 2 (32 & 64 bit) full unicode version by djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Framework/StdHdr.h"
#include "Framework/FileUtils.h"

#include "Common.h"
#include "FarPlugin.h"
#include "Engine.h"
#include "EngineTools.h"
#include "tools.h"
#include "ui.h"

void * Alloc(size_t size)
{
  size = (size / 4096 + 1) * 4096;
  return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
}

void Free(void * ptr)
{
  VirtualFree(ptr, 0, MEM_RELEASE);
}

void Compress(HANDLE handle, int f)
{
  if (f == ATTR_INHERIT) return;
  USHORT b = f ?
             COMPRESSION_FORMAT_DEFAULT :
             COMPRESSION_FORMAT_NONE;
  DWORD cb;
  if (!DeviceIoControl(handle, FSCTL_SET_COMPRESSION,
                       (LPVOID)&b, sizeof(b), NULL, 0, &cb, NULL))
    Error(LOC("Error.Compress"), GetLastError());
}

int GetCompression(HANDLE handle)
{
  USHORT res;
  DWORD cb;
  if (!DeviceIoControl(handle, FSCTL_GET_COMPRESSION,
                       NULL, 0, &res, sizeof(res), &cb, NULL))
    return -1;
  else
    return res != COMPRESSION_FORMAT_NONE;
}

void Encrypt(const String & fn, int f)
{
  if (!Win2K || f == ATTR_INHERIT) return;
  int res;
  SetFileAttributes(fn.ptr(), 0);
  if (f) res = EncryptFile(fn.ptr());
  else res = DecryptFile(fn.ptr(), 0);
  if (!res)
    Error2(LOC("Error.Encrypt"), fn, GetLastError());
}

void Encrypt(HANDLE handle, int f)
{
  if (!Win2K || f == ATTR_INHERIT) return;
  DWORD cb;
  ENCRYPTION_BUFFER enc;
  enc.EncryptionOperation = f ?
                            FILE_SET_ENCRYPTION :
                            FILE_CLEAR_ENCRYPTION;
  enc.Private[0] = 0;
  if (!DeviceIoControl(handle, FSCTL_SET_ENCRYPTION,
                       (LPVOID)&enc, sizeof(enc), NULL, 0, &cb, NULL))
    Error(LOC("Error.Encrypt"), GetLastError());
}

void _CopyACL(const String & src, const String & dst, SECURITY_INFORMATION si)
{
  const int bufSize = 16384;

  DWORD cb;
  PSECURITY_DESCRIPTOR secbuf = (PSECURITY_DESCRIPTOR)new char[bufSize];
  int res = GetFileSecurity(src.ptr(), si, secbuf, bufSize, &cb);
  if (res && cb)
  {
    delete[](char *)secbuf;
    secbuf = (PSECURITY_DESCRIPTOR)new char[cb];
    res = GetFileSecurity(src.ptr(), si, secbuf, cb, &cb);
  }
  if (res)
  {
    SetFileSecurity(dst.ptr(), si, secbuf);
  }
  delete[](char *)secbuf;
}

int SACLPriv = 0;

void CopyACL(const String & src, const String & dst)
{
  if (!SACLPriv)
  {
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkp;
    OpenProcessToken(GetCurrentProcess(),
                     TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
    LookupPrivilegeValue(NULL, SE_SECURITY_NAME, &luid
                        );
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL);
    SACLPriv = 1;
  }
  _CopyACL(src, dst, DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION);
  _CopyACL(src, dst, SACL_SECURITY_INFORMATION);
}


HANDLE Open(const String & fn, int mode, int attr)
{
  // new feature by slst: ReadFilesOpenedForWriting checking (bug #17)
  DWORD dwShareMode = FILE_SHARE_READ;
  PropertyMap & Options = plugin->Options();
  if ((mode & OPEN_READ) && (BOOL)(Options[L"ReadFilesOpenedForWriting"]))
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

  int f;
  if (mode & OPEN_READ) f = OPEN_EXISTING;
  else if (mode & OPEN_CREATE) f = CREATE_ALWAYS;
  else
    f = OPEN_ALWAYS;
  if (!(mode & OPEN_READ))
    SetFileAttributes(fn.ptr(), FILE_ATTRIBUTE_NORMAL);

  HANDLE res = CreateFile(
                 // fixed by Nsky: bug #28
                 (!fn.left(4).icmp("nul\\")) ? L"nul" : fn.ptr(),
                 // fixed by slst: bug #17
                 // mode & OPEN_READ ? (GENERIC_READ) : (GENERIC_READ | GENERIC_WRITE),
                 // fixed by slst: bug #48
                 // fix #17 is partially rolled back:
                 // Setting "compressed" attribute requires GENERIC_READ | GENERIC_WRITE
                 // access mode during file write.
                 mode & OPEN_READ ? (GENERIC_READ) : (GENERIC_READ | GENERIC_WRITE),
                 // fixed by slst: bug #12
                 // FILE_SHARE_READ | FILE_SHARE_WRITE | (WinNT ? FILE_SHARE_DELETE : 0),
                 dwShareMode,
                 NULL,
                 f,
                 (mode & OPEN_BUF ? 0 : FILE_FLAG_NO_BUFFERING) | attr,
                 NULL);
  if (res == INVALID_HANDLE_VALUE) res = NULL;
  if (res && (mode & OPEN_APPEND))
    SetFilePointer(res, 0, NULL, FILE_END);
  return res;
}


void Close(HANDLE h)
{
  CloseHandle(h);
}


int64_t FSeek(HANDLE h, int64_t pos, int method)
{
  LONG hi32 = HI32(pos), lo32 = SetFilePointer(h, LO32(pos), &hi32, method);
  if (lo32 == INVALID_SET_FILE_POINTER && GetLastError())
    return -1;
  else
    return MAKEINT64(lo32, hi32);
}

int64_t FTell(HANDLE h)
{
  return FSeek(h, 0, FILE_CURRENT);
}

void setFileSizeAndTime2(const String & fn, int64_t size, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime)
{
  HANDLE h = Open(fn, OPEN_WRITE_BUF);
  if (!h)
  {
    Error2(LOC("Error.FileOpen"), fn, GetLastError());
  }
  else
  {
    setFileSizeAndTime2(h, size, creationTime, lastAccessTime, lastWriteTime);
    Close(h);
  }
}

void setFileSizeAndTime2(HANDLE h, int64_t size, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime)
{
  FSeek(h, size, FILE_BEGIN);
  SetEndOfFile(h);
  setFileTime2(h, creationTime, lastAccessTime, lastWriteTime);
}

void setFileTime2(const String & fn, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime)
{
  HANDLE h = Open(fn, OPEN_WRITE_BUF);
  if (!h)
  {
    Error2(LOC("Error.FileOpen"), fn, GetLastError());
  }
  else
  {
    setFileTime2(h, creationTime, lastAccessTime, lastWriteTime);
    Close(h);
  }
}

void setFileTime2(HANDLE h, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime)
{
  SetFileTime(h, creationTime, lastAccessTime, lastWriteTime);
}

size_t Read(HANDLE h, void * buf, size_t size)
{
  ULONG res;
  if (!ReadFile(h, buf, (DWORD)size, &res, NULL)) return (size_t)-1;
  return res;
}

size_t Write(HANDLE h, void * buf, size_t size)
{
  ULONG res;
  if (!WriteFile(h, buf, (DWORD)size, &res, NULL)) return (size_t)-1;
  return res;
}

int GetSectorSize(const String & path)
{
  DWORD x1, x2, x3, bps;
  if (GetDiskFreeSpace(
        AddEndSlash(GetFileRoot(path)).ptr(), &x1, &bps, &x2, &x3))
    return bps;
  else
  {
    //FWError("Warning: GetSectorSize failed");
    return 4096;
  }
}
