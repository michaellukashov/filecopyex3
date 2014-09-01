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

#include "StdHdr.h"
#include "FileUtils.h"
#include "StrUtils.h"
#include "FrameworkUtils.h"

inline String ExtractFileName(const String & v)
{
  return v.substr(v.find_last_of(L"\\/") + 1);
}

inline String ExtractFilePath(const String & v)
{
  size_t pos = v.find_last_of(L"\\/");
  return (pos == (size_t)-1) ? L"" : v.substr(0, pos);
}

String ExtractFileExt(const String & v)
{
  size_t p = v.rfind('.');
  if (p == (size_t)-1 || p < v.find_last_of(L"\\/"))
  {
    return L"";
  }
  else
  {
    return v.substr(p);
  }
}

String ChangeFileExt(const String & v, const String & ext)
{
  size_t p = v.rfind('.');
  if (p == (size_t)-1 || p < v.find_last_of(L"\\/"))
  {
    return v + ext;
  }
  else
  {
    return v.substr(0, p) + ext;
  }
}

inline String CutEndSlash(const String & v)
{
  if (v[v.len() - 1] == '\\' || v[v.len() - 1] == '/')
  {
    return v.left(v.len() - 1);
  }
  else
  {
    return v;
  }
}

String AddEndSlash(const String & v)
{
  if (v.len() == 0)
  {
    return L"\\";
  }
  wchar_t endChar = v[v.len() - 1];
  if (endChar == '\\' || endChar == '/')
  {
    return v;
  }
  else
  {
    return v + L"\\";
  }
}

String GetFileNameRoot(const String & v)
{
  String l = v.left(2);
  if (v.left(3) == L"\\\\.")
  {
    if (l == L"\\\\" || l == L"//")
    {
      l = v.substr(2);
      size_t p = l.find_first_of(L"\\/");
      if (p != (size_t)-1)
      {
        l = l.substr(p + 1);
        p = l.find_first_of(L"\\/");
        if (p != -1)
        {
          return l.substr(0, p + 1);
        }
      }
    }
  }
  else if (l == L"\\\\")
  {
    l = v.substr(2);
    size_t p = l.find_first_of(L"\\/");
    if (p != (size_t)-1)
    {
      l = l.substr(p + 1);
      size_t p2 = l.find_first_of(L"\\/");
      if (p2 != (size_t)-1)
      {
        return v.substr(0, p + p2 + 2 + 2);
      }
      else
      {
        return v + L"\\";
      }
    }
  }
  else if (l.find(':') == 1)
  {
    return l + L"\\";
  }
  return L"";
}

//typedef
//BOOL (WINAPI * _GetVolumeNameForVolumeMountPoint)(
//  LPCTSTR lpszVolumeMountPoint,
//  LPTSTR lpszVolumeName,
//  DWORD cchBufferLength
//);
//typedef
//BOOL (WINAPI * _GetVolumePathName)(
//  LPCTSTR lpszFileName,
//  LPTSTR lpszVolumePathName,
//  DWORD cchBufferLength
//);

//extern _GetVolumePathName pGetVolumePathName;
//static HINSTANCE hKernel=GetModuleHandle(_T("kernel32.dll"));
//
//#ifdef UNICODE
//  static _GetVolumeNameForVolumeMountPoint pGetVolumeNameForVolumeMountPoint =
//    (_GetVolumeNameForVolumeMountPoint)GetProcAddress(hKernel, "GetVolumeNameForVolumeMountPointW");
//  static _GetVolumePathName pGetVolumePathName =
//    (_GetVolumePathName)GetProcAddress(hKernel, "GetVolumePathNameW");
//#else
//  static _GetVolumeNameForVolumeMountPoint pGetVolumeNameForVolumeMountPoint =
//    (_GetVolumeNameForVolumeMountPoint)GetProcAddress(hKernel, "GetVolumeNameForVolumeMountPointA");
//  static _GetVolumePathName pGetVolumePathName =
//    (_GetVolumePathName)GetProcAddress(hKernel, "GetVolumePathNameA");
//#endif


// Added new function
// Returns primary volume mount point for other volume mount points
// E.g.: E:\ is mounted D:\mnt\drive_e\ and
// function returns "E:\" for "D:\mnt\drive_e\"
bool GetPrimaryVolumeMountPoint(const String & VolumeMountPointForPath,
                                String & PrimaryVolumeMountPoint)
{
  bool result = false;
  wchar_t VolumeNameForPath[MAX_FILENAME];

  if (!Win2K)
    return result;
  DWORD attr = ::GetFileAttributes(VolumeMountPointForPath.ptr());
  if (attr == INVALID_FILE_ATTRIBUTES)
    return result;
  if (!(attr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)))
    return result;

  if (::GetVolumeNameForVolumeMountPoint(AddEndSlash(VolumeMountPointForPath).ptr(),
                                       VolumeNameForPath,
                                       LENOF(VolumeNameForPath)))
  {
    wchar_t VolumeMountPoint[] = L"?:\\";
    wchar_t VolumeName[MAX_FILENAME];

    for (char drive = 'A'; drive < 'Z'; drive++)
    {
      VolumeMountPoint[0] = drive;
      if (::GetVolumeNameForVolumeMountPoint(VolumeMountPoint,
                                           VolumeName,
                                           LENOF(VolumeName)))
      {
        if (wcscmp(VolumeNameForPath, VolumeName) == 0)
        {
          String tmp = CutEndSlash(VolumeMountPoint);
          if (tmp != CutEndSlash(VolumeMountPointForPath))
          {
            PrimaryVolumeMountPoint = tmp;
            result = true;
          }
          break;
        }
      }
    }
  }

  return result;
}


bool GetSymLink(const String & _dir, String & res, int flg)
{
  res.Clear();
  String dir = CutEndSlash(_dir);
  wchar_t buf[MAX_FILENAME];
  DWORD sz = LENOF(buf);
  res = dir;

  if (flg & gslExpandSubst && (dir.len() == 2) && (dir[1] == ':') &&
      ::QueryDosDevice(dir.ptr(), buf, LENOF(buf)) > 0)
  {
    String r = buf;
    if (r.left(8) == L"\\??\\UNC\\")
    {
      res = CutEndSlash(String(L"\\\\") + r.substr(8));
      return true;
    }
    if (r.left(4) == L"\\??\\")
    {
      res = CutEndSlash(r.substr(4));
      return true;
    }
  }

  // ::GetFileAttributes returns FILE_ATTRIBUTE_REPARSE_POINT
  // for drives names e.g. "E:"
  // Symlinks should have length > 2
  if (Win2K && (flg & gslExpandReparsePoints) && dir.len() > 2)
  {
    DWORD attr = ::GetFileAttributes(dir.ptr());
    if (attr != INVALID_FILE_ATTRIBUTES &&
        attr & FILE_ATTRIBUTE_DIRECTORY &&
        attr & FILE_ATTRIBUTE_REPARSE_POINT)
    {
      HANDLE hf = ::CreateFile(dir.ptr(),
                             GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                             nullptr,
                             OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
                             nullptr);
      if (hf != INVALID_HANDLE_VALUE)
      {
        uint8_t Data[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
        ::ZeroMemory(Data, sizeof(Data));
        DWORD returnedLength = 0;
        if (::DeviceIoControl(hf, FSCTL_GET_REPARSE_POINT, nullptr, 0,
                            Data, sizeof(Data), &returnedLength, nullptr))
        {
          REPARSE_GUID_DATA_BUFFER * rd = (REPARSE_GUID_DATA_BUFFER *)Data;
          if ((IsReparseTagMicrosoft(rd->ReparseTag)) &&
              (rd->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) &&
              (!memcmp(rd->ReparseGuid.Data4, "\\??\\", 4)))
          {
            String r = (wchar_t *)&rd->GenericReparseBuffer;
            if (r.left(7) != L"Volume{")
            {
              ::CloseHandle(hf);
              res = CutEndSlash(buf);
              return true;
            }
          }
        }
        ::CloseHandle(hf);
      }
    }
  }

  if (Win2K && (flg & gslExpandMountPoints))
  {
    //if (pGetVolumeNameForVolumeMountPoint(AddEndSlash(dir).ptr(), buf, LENOF(buf)))
    if (GetPrimaryVolumeMountPoint(dir, res))
    {
      return true;
    }
  }

  if (flg & gslExpandNetMappings && (dir.len() == 2) && (dir[1] == ':') &&
      WNetGetConnection(dir.ptr(), buf, &sz) == 0)
  {
    res = CutEndSlash(buf);
    return true;
  }

  return false;
}


String GetRealFileName(const String & _path, int flg)
{
  String path = AddEndSlash(_path);
rep:
  for (intptr_t Index = path.len() - 1; Index >= 0; Index--)
  {
    if (path[Index] == '\\')
    {
      String res;
      if (GetSymLink(path.substr(0, Index), res,
                     gslExpandSubst | gslExpandReparsePoints | gslExpandMountPoints |
                     ((flg & rfnNoNetExpand) ? 0 : gslExpandNetMappings)))
      {
        path = AddEndSlash(res + path.substr(Index));
        goto rep;
      }
    }
  }
  return CutEndSlash(path);
}

String GetFileRoot(const String & _path)
{
  String path = GetRealFileName(_path), res;
  if (Win2K)
  {
    wchar_t buf[MAX_FILENAME];
    if (GetVolumePathName(path.ptr(), buf, LENOF(buf)))
      res = AddEndSlash(buf);
    else
      res = GetFileNameRoot(path);
  }
  else
    res = GetFileNameRoot(path);
  return res;
}

String ExpandEnv(const String & v)
{
  wchar_t buf[MAX_FILENAME];
  ExpandEnvironmentStrings(v.ptr(), buf, LENOF(buf));
  return buf;
}

String ApplyFileMask(const String & _name, const String & _mask)
{
  const wchar_t * name = _name.c_str();
  const wchar_t * next = name + _name.len() - 1;
  String strName;
  String strNameExt;
  while (next >= name && *next != '.')
    next--;
  if (next < name)
  {
    strName = _name;
  }
  else
  {
    strName = String(name, next - name);
    strNameExt = String(++next);
  }

  const wchar_t * mask = _mask.c_str();
  const wchar_t * mext = mask + _mask.len() - 1;
  String strMask;
  String strMaskExt;
  while (mext >= mask && *mext != '.')
    mext--;
  if (mext < mask)
  {
    strMask = _mask;
  }
  else
  {
    strMask = String(mask, mext - mask);
    strMaskExt = String(++mext);
  }

  wchar_t sym = L'\0';
  String res;

  for (size_t i = 0; i < strMask.len(); i++)
  {
    if (strMask[i] == L'*')
    {
      res += (sym == L'\0') ? strName : strNameExt;
    }
    else
    {
      if (strMask[i] == L'?')
        sym = (i < strName.len()) ? strName[i] : L'\0';
      else
        sym = strMask[i];
      if (sym != L'\0')
        res += sym;
    }
  }

  if (!strMaskExt.empty())
  {
    res += L'.';
    for (size_t i = 0; i < strMaskExt.len(); i++)
    {
      if (strMaskExt[i] == L'*')
      {
        res += strNameExt;
      }
      else if (strMaskExt[i] == L'?')
      {
        if (i < strNameExt.len())
          res += strNameExt[i];
      }
      else
      {
        res += strMaskExt[i];
      }
    }
  }
  return res;
}

String ApplyFileMaskPath(const String & name, const String & mask)
{
  if (mask[mask.len() - 1] == '\\' || mask[mask.len() - 1] == '/')
    return mask + ExtractFileName(name);
  DWORD a = ::GetFileAttributes(mask.ptr());
  if (a != INVALID_FILE_ATTRIBUTES && a & FILE_ATTRIBUTE_DIRECTORY)
  {
    String res = mask;
    if (name.icmp(mask))
      res += String(L"\\") + ExtractFileName(name);
    return res;
  }
  return AddEndSlash(ExtractFilePath(mask)) +
         ApplyFileMask(ExtractFileName(name), ExtractFileName(mask));
}

bool FileExists(const String & name)
{
  return (::GetFileAttributes(name.ptr()) != INVALID_FILE_ATTRIBUTES);
}

int64_t FileSize(HANDLE h)
{
  ULONG hsz, lsz = ::GetFileSize(h, &hsz);
  return MAKEINT64(lsz, hsz);
}

int64_t FileSize(const String & fn)
{
  WIN32_FIND_DATA fd;
  HANDLE h = ::FindFirstFile(fn.ptr(), &fd);
  if (h != INVALID_HANDLE_VALUE)
  {
    ::FindClose(h);
    return MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh);
  }
  else
    return 0;
}

inline String TempName()
{
  srand((uint32_t)time(nullptr));
  return Format(L"%8.8x", rand() * rand() * rand()) + L"." + Format(L"%3.3x", rand());
}

String TempPath()
{
  wchar_t buf[MAX_FILENAME];
  ::GetTempPath(LENOF(buf), buf);
  return CutEndSlash(buf);
}

String TempPathName()
{
  return AddEndSlash(TempPath()) + TempName();
}

static bool MoveFile2(const wchar_t * src, const wchar_t * dst)
{
  DWORD attr = ::GetFileAttributes(dst);
  ::SetFileAttributes(dst, FILE_ATTRIBUTE_NORMAL);
  if (::MoveFile(src, dst))
    return true;
  else
  {
    DWORD err = ::GetLastError();
    ::SetFileAttributes(dst, attr);
    ::SetLastError(err);
    return false;
  }
}

static bool __MoveFileEx(const wchar_t * src, const wchar_t * dst, uint32_t flg)
{
  DWORD attr = ::GetFileAttributes(dst);
  if (_wcsicmp(src, dst) != 0)
    ::SetFileAttributes(dst, FILE_ATTRIBUTE_NORMAL);
  if (::MoveFileEx(src, dst, flg))
    return true;
  else
  {
    DWORD err = ::GetLastError();
    ::SetFileAttributes(dst, attr);
    ::SetLastError(err);
    return false;
  }
}

bool MoveFile(const String & _src, const String & _dst, intptr_t replace)
{
  // return false if dst is hard link
  if (WinNT && replace)
  {
    HANDLE DstFileHandle = Open(_dst, OPEN_READ, 0);
    if (DstFileHandle == nullptr)
      return false;
    BY_HANDLE_FILE_INFORMATION FileInformation;
    if (::GetFileInformationByHandle(DstFileHandle, &FileInformation))
    {
      Close(DstFileHandle);
      if (!(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
          (FileInformation.nNumberOfLinks > 1))
      {
        return false; // Hardlinks are replaced by FileCopyEx engine
      }
    }
    else
    {
      Close(DstFileHandle);
      return false; // cannot get info by handle
    }
  }

  ForceDirectories(_dst);
  if (WinNT)
  {
    return __MoveFileEx(_src.ptr(), _dst.ptr(), replace ? MOVEFILE_REPLACE_EXISTING : 0);
  }
  else
  {
    String root1 = GetFileRoot(_src);
    String root2 = GetFileRoot(_dst);
    if (root1.IsEmpty() || root2.IsEmpty())
    {
      ::SetLastError(ERROR_PATH_NOT_FOUND);
      return false;
    }
    if (!root1.icmp(root2))
    {
      String src = GetRealFileName(_src);
      String dst = GetRealFileName(_dst);
      if (!src.icmp(dst))
        return true;
      DWORD sa = ::GetFileAttributes(src.ptr());
      if (sa == INVALID_FILE_ATTRIBUTES)
      {
        ::SetLastError(ERROR_FILE_NOT_FOUND);
        return false;
      }
      DWORD da = ::GetFileAttributes(dst.ptr());
      if (da != INVALID_FILE_ATTRIBUTES)
      {
        if (sa & FILE_ATTRIBUTE_DIRECTORY || da & FILE_ATTRIBUTE_DIRECTORY)
        {
          ::SetLastError(ERROR_ACCESS_DENIED);
          return false;
        }
        if (!replace)
        {
          ::SetLastError(ERROR_ALREADY_EXISTS);
          return false;
        }
        String temp = AddEndSlash(ExtractFilePath(dst)) + TempName();
        if (!MoveFile2(dst.ptr(), temp.ptr()))
          return false;
        if (!MoveFile2(src.ptr(), dst.ptr()))
        {
          DWORD err = ::GetLastError();
          MoveFile2(temp.ptr(), dst.ptr());
          ::SetLastError(err);
          return false;
        }
        ::SetFileAttributes(temp.ptr(), FILE_ATTRIBUTE_NORMAL);
        ::DeleteFile(temp.ptr());
        return true;
      }
      return MoveFile2(src.ptr(), dst.ptr());
    }
    else
    {
      ::SetLastError(ERROR_NOT_SAME_DEVICE);
      return false;
    }
  }
}

void ForceDirectories(const String & s)
{
  wchar_t * ptr = (wchar_t *)s.ptr();
  wchar_t * sptr = ptr;
  while (*ptr)
  {
    if (*ptr == '\\' || *ptr == '/')
    {
      wchar_t t = *ptr;
      *ptr = 0;
      ::CreateDirectory(sptr, nullptr);
      *ptr = t;
    }
    ptr++;
  }
}

void Out(const String & s)
{
  DWORD cb;
  ::WriteConsole(::GetStdHandle(STD_OUTPUT_HANDLE), (const void *)s.ptr(),
               (DWORD)s.len(), &cb, nullptr);
}


void Close(HANDLE h)
{
  ::CloseHandle(h);
}

bool Delete(const String & fn)
{
  DWORD attr = ::GetFileAttributes(fn.ptr());
  ::SetFileAttributes(fn.ptr(), FILE_ATTRIBUTE_NORMAL);
  if (!::DeleteFile(fn.ptr()))
  {
    ::SetFileAttributes(fn.ptr(), attr);
    return false;
  }
  return true;
}
