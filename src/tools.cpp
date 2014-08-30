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
#include "tools.h"

// special HDD ID implementation for NT4
#define HARDDISK_CONST_PART         _T("\\Device\\Harddisk")
#define HARDDISK_CONST_PART_LEN     (LENOF(HARDDISK_CONST_PART)-1)

int GetDriveId(const String & path, String & res)
{
  wchar_t buf[MAX_FILENAME];
  if (WinNT4)
  {
    // special HDD ID implementation for NT4
    if (QueryDosDevice(path.left(2).ptr(), buf, LENOF(buf)) > 0)
    {
      String s = buf;
      String tmp = L"\\Device\\Harddisk";
      if (!s.nicmp(tmp, tmp.len()))
      {
        res = s.substr(tmp.len(), 1);
        return TRUE;
      }
    }
  }
  else if (QueryDosDevice(path.left(2).ptr(), buf, LENOF(buf)) > 0)
  {
    String s = buf;
    String tmp = L"\\Device\\HarddiskDmVolumes\\";
    if (!s.nicmp(tmp, tmp.len()))
    {
      s = s.substr(tmp.len());
      res = s.left(s.find('\\'));
      return TRUE;
    }
  }
  return FALSE;
}

static int GetPhysDrive(const String & _path, int & res)
{
  String path = _path;
  if (Win2K || WinXP)
  {
    if (path.left(11) != L"\\\\?\\Volume{")
    {
      wchar_t buf[MAX_FILENAME];
      if (!GetVolumeNameForVolumeMountPoint(path.ptr(), buf, LENOF(buf)))
        return FALSE;
      path = buf;
    }
    HANDLE hVolume = ::CreateFile(CutEndSlash(path).ptr(), 0,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                0, nullptr);
    if (hVolume == INVALID_HANDLE_VALUE)
      return FALSE;

    char outbuf[1024];
    DWORD ret;
    if (::DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, 0,
                        outbuf, LENOF(outbuf), &ret, nullptr))
    {
      VOLUME_DISK_EXTENTS * ext = (VOLUME_DISK_EXTENTS *)outbuf;
      res = ext->Extents[0].DiskNumber;
      ::CloseHandle(hVolume);
      return TRUE;
    }
    ::CloseHandle(hVolume);
  }

  return FALSE;
}

#define FILE_READ_ONLY_VOLUME 0x00080000
#define FILE_NAMED_STREAMS    0x00040000

uint32_t VolFlags(const String & _path)
{
  String path = _path;
  size_t sl = path.find_last_of(L"\\/");
  size_t ml = path.find_first_of(L"*?");
  if (ml != (size_t)-1 && ml < sl)
    return -1;
  if (path.find_first_of(L"|<>") != (size_t)-1)
    return -1;
  size_t cl = path.find(':');
  if (cl != (size_t)-1 && (cl != 1 || cl != path.rfind(':') ||
                          (cl != path.len() - 1 && path.find_first_of(L"\\/") != 2)))
    return -1;
  if (ml != (size_t)-1)
    path = ExtractFilePath(path);
  String root = GetFileRoot(path);

  DWORD clen, flg;
  wchar_t sysname[32];
  if (GetVolumeInformation(root.ptr(), nullptr, 0, nullptr, &clen, &flg, sysname, LENOF(sysname)))
  {
    uint32_t res = 0;
    if (flg & FILE_FILE_COMPRESSION)
      res |= VF_COMPRESSION;
    if (flg & FILE_SUPPORTS_ENCRYPTION)
      res |= VF_ENCRYPTION;
    if (flg & FILE_PERSISTENT_ACLS)
      res |= VF_RIGHTS;
    if (flg & FILE_NAMED_STREAMS)
      res |= VF_STREAMS;
    // NT4 supports streams, but has no special flag for this fact
    if (!_wcsicmp(sysname, TEXT("NTFS")))
      res |= VF_STREAMS;
    if (flg & FILE_READ_ONLY_VOLUME)
      res |= VF_READONLY;
    if (flg & FILE_UNICODE_ON_DISK)
      res |= VF_UNICODE;
    if (GetDriveType(root.ptr()) == DRIVE_CDROM)
      res |= VF_CDROM;
    return res;
  }
  return -1;
}

intptr_t CheckParallel(const String & _srcpath, const String & _dstpath)
{
  String root1 = GetFileRoot(_srcpath);
  String root2 = GetFileRoot(_dstpath);
  int srctype = GetDriveType(root1.ptr());
  int dsttype = GetDriveType(root2.ptr());
  if (srctype != dsttype)
    return TRUE;
  if (!root1.icmp(root2))
    return FALSE;

  if (srctype == DRIVE_REMOTE)
    return FALSE;
  else if (srctype == DRIVE_CDROM || srctype == DRIVE_REMOVABLE)
    return TRUE;
  else if (srctype == DRIVE_FIXED)
  {
    int drv1, drv2;
    if (GetPhysDrive(root1, drv1) &&
        GetPhysDrive(root2, drv2))
    {
      if (drv1 != drv2)
        return TRUE;
      else
        return FALSE;
    }
    String id1, id2;
    if (GetDriveId(root1, id1) &&
        GetDriveId(root2, id2))
    {
      if (id1.icmp(id2))
        return TRUE;
      else
        return FALSE;
    }
  }

  return -1;
}

String DupName(const String & src, intptr_t n)
{
  return ChangeFileExt(src, L"") + L"_" + String(n) + ExtractFileExt(src);
}

intptr_t ExistsN(const String & fn, intptr_t n)
{
  if (!n)
    return FileExists(fn);
  else
    return FileExists(DupName(fn, n));
}

bool Newer(const FILETIME & ft1, const FILETIME & ft2)
{
  return CompareFileTime(&ft1, &ft2) >= 0;
}

bool Newer(const String & fn1, const FILETIME & ft2)
{
  WIN32_FIND_DATA fd;
  HANDLE hf = ::FindFirstFile(fn1.ptr(), &fd);
  if (hf != INVALID_HANDLE_VALUE)
  {
    ::FindClose(hf);
    return Newer(fd.ftLastWriteTime, ft2);
  }
  return FALSE;
}

intptr_t RmDir(const String & fn)
{
  DWORD attr = ::GetFileAttributes(fn.ptr());
  ::SetFileAttributes(fn.ptr(), FILE_ATTRIBUTE_NORMAL);
  if (!::RemoveDirectory(fn.ptr()))
  {
    ::SetFileAttributes(fn.ptr(), attr);
    return FALSE;
  }
  return TRUE;
}

void DebugLog(const wchar_t * DebugMsg, ...)
{
  wchar_t MsgBuf[0x400];
  va_list ArgPtr;
  va_start(ArgPtr, DebugMsg);
  _vsnwprintf_s(MsgBuf, LENOF(MsgBuf), LENOF(MsgBuf), DebugMsg, ArgPtr);
  va_end(ArgPtr);
  OutputDebugString(MsgBuf);
}

String convertPath(enum CONVERTPATHMODES mode, const String & src)
{
  wchar_t fullName[MAX_PATH];
  size_t size = FSF.ConvertPath(mode, src.c_str(), fullName, sizeof(fullName));
  if (size <= MAX_PATH)
  {
    String res(fullName);
    return res;
  }
  else
  {
    wchar_t * fullNameBuf = new wchar_t[size];
    FSF.ConvertPath(mode, src.c_str(), fullNameBuf, size);
    String res(fullNameBuf);
    delete[] fullNameBuf;
    return res;
  }
}
