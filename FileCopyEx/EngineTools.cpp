#include <stdhdr.h>

#include "common.h"
#include "api.h"
#include "FileCopyEx.h"
#include "engine.h"
#include "EngineTools.h"

void* Alloc(int size)
{
  size = (size/4096+1)*4096;
  return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
}

void Free(void *ptr)
{
  VirtualFree(ptr, 0, MEM_RELEASE);
}

void Compress(HANDLE handle, int f)
{
  if (f==ATTR_INHERIT) return;
  USHORT b = f?
    COMPRESSION_FORMAT_DEFAULT:
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
    return res!=COMPRESSION_FORMAT_NONE;
}

void Encrypt(const String& fn, int f)
{
  if (!Win2K || f==ATTR_INHERIT) return;
  int res;
  SetFileAttributes(fn.ptr(), 0);
  if (f) res = pEncryptFile(fn.ptr());
  else res = pDecryptFile(fn.ptr(), 0);
  if (!res)
    Error2(LOC("Error.Encrypt"), fn, GetLastError());
}

void Encrypt(HANDLE handle, int f)
{
  if (!Win2K || f==ATTR_INHERIT) return;
  DWORD cb;
  ENCRYPTION_BUFFER enc;
  enc.EncryptionOperation=f? 
    FILE_SET_ENCRYPTION:
    FILE_CLEAR_ENCRYPTION;
  enc.Private[0]=0;
  if (!DeviceIoControl(handle, FSCTL_SET_ENCRYPTION,
    (LPVOID)&enc, sizeof(enc), NULL, 0, &cb, NULL))
      Error(LOC("Error.Encrypt"), GetLastError());
}

void *_ReadACL(const String& fn, SECURITY_INFORMATION si)
{
  DWORD cb;
  PSECURITY_DESCRIPTOR secbuf = (PSECURITY_DESCRIPTOR)malloc(16384);
  int res=pGetFileSecurity(fn.ptr(), si, secbuf, 16384, &cb);
  if (res && cb)
  {
    secbuf = (PSECURITY_DESCRIPTOR)realloc(secbuf, cb);
    res=pGetFileSecurity(fn.ptr(), si, secbuf, 16384, &cb);
  }
  if (res) return secbuf;
  else return NULL;
}

void  _WriteACL(const String& fn, SECURITY_INFORMATION si, void *buf)
{
  if (buf)
    pSetFileSecurity(fn.ptr(), si, buf);
}

void ReadACL(const String& fn, void **info1, void **info2)
{
  *info1=_ReadACL(fn, DACL_SECURITY_INFORMATION 
    | GROUP_SECURITY_INFORMATION
    | OWNER_SECURITY_INFORMATION);
  *info2=_ReadACL(fn, SACL_SECURITY_INFORMATION);
}

void WriteACL(const String& fn, void *info1, void *info2)
{
  _WriteACL(fn, DACL_SECURITY_INFORMATION 
    | GROUP_SECURITY_INFORMATION
    | OWNER_SECURITY_INFORMATION, info1);
  _WriteACL(fn, SACL_SECURITY_INFORMATION, info2);
}

void _CopyACL(const String& src, const String& dst, SECURITY_INFORMATION si)
{
  DWORD cb;
  PSECURITY_DESCRIPTOR secbuf = (PSECURITY_DESCRIPTOR)malloc(16384);
  int res=pGetFileSecurity(src.ptr(), si, secbuf, 16384, &cb);
  if (res && cb)
  {
    secbuf = (PSECURITY_DESCRIPTOR)realloc(secbuf, cb);
    res=pGetFileSecurity(src.ptr(), si, secbuf, 16384, &cb);
  }
  if (res)
    pSetFileSecurity(dst.ptr(), si, secbuf);
  free(secbuf);
}

int SACLPriv = 0;

void CopyACL(const String& src, const String& dst)
{
  if (!SACLPriv)
  {
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkp;
    pOpenProcessToken(GetCurrentProcess(), 
      TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
    pLookupPrivilegeValue(NULL, SE_SECURITY_NAME, &luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    pAdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL);
    SACLPriv = 1;
  }
  _CopyACL(src, dst, DACL_SECURITY_INFORMATION 
    | GROUP_SECURITY_INFORMATION
    | OWNER_SECURITY_INFORMATION);
  _CopyACL(src, dst, SACL_SECURITY_INFORMATION);
}


HANDLE Open(const String& fn, int mode, int attr)
{
  // new feature by slst: ReadFilesOpenedForWriting checking (bug #17)
  DWORD dwShareMode = FILE_SHARE_READ;

  if ((mode & OPEN_READ) && (BOOL)(Instance->Options["ReadFilesOpenedForWriting"]))
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  
  int f;
  if (mode & OPEN_READ) f = OPEN_EXISTING;
  else 
    if (mode & OPEN_CREATE) f = CREATE_ALWAYS;
    else 
      f = OPEN_ALWAYS;
  if (!(mode & OPEN_READ))
    SetFileAttributes(fn.ptr(), FILE_ATTRIBUTE_NORMAL);

  HANDLE res = CreateFile(
    // fixed by Nsky: bug #28
    (!fn.left(4).icmp("nul\\")) ? _T("nul") : fn.ptr(),
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
    (mode & OPEN_BUF ? 0: FILE_FLAG_NO_BUFFERING) | attr, 
    NULL);
  if (res==INVALID_HANDLE_VALUE) res=NULL;
  if (res && (mode & OPEN_APPEND))
    SetFilePointer(res, 0, NULL, FILE_END);
  return res;
}


void Close(HANDLE h)
{
  CloseHandle(h);
}


__int64 FSeek(HANDLE h, __int64 pos, int method)
{
  LONG hi32=HI32(pos), lo32=SetFilePointer(h, LO32(pos), &hi32, method);
  if (lo32==INVALID_SET_FILE_POINTER && GetLastError())
    return -1;
  else
    return MAKEINT64(lo32, hi32);
}

__int64 FTell(HANDLE h)
{
  return FSeek(h, 0, FILE_CURRENT);
}

void SetFileSizeAndTime(const String& fn, __int64 size, FILETIME& time)
{
  HANDLE h=Open(fn, OPEN_WRITE_BUF);
  if (!h)
    Error2(LOC("Error.FileOpen"), fn, GetLastError());
  else
  {
    FSeek(h, size, FILE_BEGIN);
    SetEndOfFile(h);
    SetFileTime(h, NULL, NULL, &time);
    Close(h);
  }
}

void SetFileSizeAndTime(HANDLE h, __int64 size, FILETIME& time)
{
  FSeek(h, size, FILE_BEGIN);
  SetEndOfFile(h);
  SetFileTime(h, NULL, NULL, &time);
}

void SetFileTime(const String& fn, FILETIME& time)
{
  HANDLE h=Open(fn, OPEN_WRITE_BUF);
  if (!h)
    Error2(LOC("Error.FileOpen"), fn, GetLastError());
  else
  {
    SetFileTime(h, NULL, NULL, &time);
    Close(h);
  }
}

void SetFileTime(HANDLE h, FILETIME& time)
{
  SetFileTime(h, NULL, NULL, &time);
}

int Read(HANDLE h, void *buf, int size)
{
  ULONG res;
  if (!ReadFile(h, buf, size, &res, NULL)) return -1;
  return res;
}

int Write(HANDLE h, void *buf, int size)
{
  ULONG res;
  if (!WriteFile(h, buf, size, &res, NULL)) return -1;
  return res;
}

void ClearInput()
{
  HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
  while (1)
  {
    INPUT_RECORD rec;
    DWORD rc;
    PeekConsoleInput(h, &rec, 1, &rc);
    if (!rc) break;
    ReadConsoleInput(h, &rec, 1, &rc);
  }
}


int Engine::EngineError(const String& s, const String& fn, int code, int& flg,
                        const String& title, const String& type_id)
{
  int ix=-1;
  if (flg & eeAutoSkipAll)
  {
    ix=errTypes.Find(type_id);
    if (ix==-1) ix=errTypes.Add(type_id);
    if (errTypes.Values(ix) & errfSkipAll) 
    {
      if (flg & eeShowKeepFiles && errTypes.Values(ix) & errfKeepFiles) 
        flg |= eerKeepFiles;
      return RES_SKIP;
    }
  }

  FarDialog &dlg=Instance->Dialogs["CopyError"];
  dlg.ResetControls();
  if (title!="") dlg("Title")=title;
  
  if (flg & eeShowReopen) dlg["Reopen"]("Visible")=1;
  if (flg & eeShowKeepFiles) dlg["KeepFiles"]("Visible")=1;
  if (flg & eeYesNo) dlg["YesNo"]("Visible")=1;
  if (flg & eeRetrySkipAbort) 
  {
    dlg["RetrySkipAbort"]("Visible")=1;
    if (flg & eeAutoSkipAll) dlg["SkipAll"]("Visible")=1;
  }
  dlg["Label1"]("Text")=s;
  dlg["Label1"]("Visible")=1;
  if (flg & eeOneLine)
  {
    dlg["Sep1"]("Visible")=0;
  }
  else
  {
    dlg["Label2"]("Text")=FormatWidthNoExt(fn, msgw());
    dlg["Label2"]("Visible")=1;
    StringList list;
    list.LoadFromString(SplitWidth(GetErrText(code), msgw()), '\n');
    for (int i=0; i<list.Count(); i++)
    {
      if (i<=7)
      {
        String name="Label"+String(i+3);
        dlg[name]("Visible")=1;
        dlg[name]("Text")=list[i];
      }
    }
  }

  if (!(flg & eeShowReopen) && !(flg & eeShowKeepFiles))
  {
    dlg["Sep1"]("Visible")=0;
  }

  int res=dlg.Execute();

  if ((bool)dlg["Reopen"]("Selected")) flg |= eerReopen;
  if ((bool)dlg["KeepFiles"]("Selected")) flg |= eerKeepFiles;

  if (flg & eeYesNo) 
  {
    if (res==0) return RES_YES;
    else if (res==-1) return RES_NO;
  }
  else if (flg & eeRetrySkipAbort) 
  {
    if (res==0) return RES_RETRY;
    else if (res==1) return RES_SKIP;
    else if (res==-1) return RES_ABORT;
    else if (res==2)
    {
      if (ix!=-1) 
      {
        errTypes.Values(ix) |= errfSkipAll;
        if (flg & eerKeepFiles) errTypes.Values(ix) |= errfKeepFiles;
      }
      return RES_SKIP;
    }
  }
  return -1;
}

int GetSectorSize(const String& path)
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
