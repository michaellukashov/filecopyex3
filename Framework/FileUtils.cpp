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

#include "stdhdr.h"
#include "fileutils.h"
#include "strutils.h"
#include "fwcommon.h"

#include "..\src\Common.h"
#include "..\src\FileCopyEx.h"
#include "..\src\engine.h"
#include "..\src\EngineTools.h"

inline String ExtractFileName(const String& v)
{
  return v.substr(v.crfind("\\/")+1);
}

inline String ExtractFilePath(const String& v)
{
  return v.substr(0, v.crfind("\\/"));
}

String ExtractFileExt(const String& v)
{
  int p=v.crfind('.');
  if (p==-1 || p<v.crfind("\\/")) return "";
  else return v.substr(p);
}

String ChangeFileExt(const String& v, const String& ext)
{
  int p=v.crfind('.');
  if (p==-1 || p<v.crfind("\\/")) return v+ext;
  else return v.substr(0, p)+ext;
}

inline String CutEndSlash(const String& v)
{
  if (v[v.len()-1]=='\\' || v[v.len()-1]=='/')
    return v.left(v.len()-1);
  else
    return v;
}

inline String AddEndSlash(const String& v)
{
  if (v[v.len()-1]=='\\' || v[v.len()-1]=='/')
    return v;
  else
    return v+"\\";
}

String GetFileNameRoot(const String& v)
{
  String l=v.left(2);
  // Bug #12 fixed by axxie
  if (v.left(3) == "\\\\.")
  {
    if (l=="\\\\" || l=="//")
    {
      l=v.substr(2); 
      int p=l.cfind("\\/");
      if (p!=-1)
      {
        l=l.substr(p+1);
        p=l.cfind("\\/");
        if (p!=-1)
          return l.substr(0, p+1);
      }
    }
  }
  else if (l=="\\\\")
  {
    l=v.substr(2); 
    int p=l.cfind("\\/");
    if (p!=-1)
    {
      l=l.substr(p+1);
      int p2=l.cfind("\\/");
      // bug #12 refixed by axxie
      if (p2!=-1)
        return v.substr(0,p+p2+2+2);
      else return v+"\\";
    }
  }
  else if (l.cfind(':')==1) return l+"\\";
  return "";
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


// bugfixed by slst: bug #34
// Added new function
// Returns primary volume mount point for other volume mount points
// E.g.: E:\ is mounted D:\mnt\drive_e\ and 
// function returns "E:\" for "D:\mnt\drive_e\" 
BOOL GetPrimaryVolumeMountPoint(const String& VolumeMountPointForPath,
                                String& PrimaryVolumeMountPoint)
{
  BOOL result = FALSE;
  wchar_t VolumeNameForPath[MAX_FILENAME];

  if (!Win2K) return result;
  int attr = GetFileAttributes(VolumeMountPointForPath.ptr());
  if (attr == 0xFFFFFFFF) return result;
  if (!(attr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) return result;

  if (GetVolumeNameForVolumeMountPoint(AddEndSlash(VolumeMountPointForPath).ptr(),
                                        VolumeNameForPath,
                                        sizeof(VolumeNameForPath)/sizeof(wchar_t)))
  {
    wchar_t VolumeMountPoint[] = _T("?:\\");
    wchar_t VolumeName[MAX_FILENAME];

    for (char drive = 'A'; drive < 'Z'; drive++)
    {
      VolumeMountPoint[0] = drive;
      if (GetVolumeNameForVolumeMountPoint(VolumeMountPoint,
                                            VolumeName,
                                            sizeof(VolumeName)/sizeof(wchar_t)))
      {
        if (_tcscmp(VolumeNameForPath, VolumeName) == 0)
        {
          String tmp = CutEndSlash(VolumeMountPoint);
          if (tmp != CutEndSlash(VolumeMountPointForPath))
          {
            PrimaryVolumeMountPoint = tmp;
            result = TRUE;
          }
          break;
        }
      }
    }
  }

  return result;
}


int GetSymLink(const String &_dir, String &res, int flg)
{
  res = "";
  String dir=CutEndSlash(_dir);
  wchar_t buf[MAX_FILENAME];
  DWORD sz=MAX_FILENAME;
  res=dir;

  if (flg & gslExpandSubst && (dir.len()==2) && (dir[1]==':') && 
      QueryDosDevice(dir.ptr(), buf, MAX_FILENAME)>0)
  {
    String r=buf;
    if (r.left(8)=="\\??\\UNC\\") 
    {
      res=CutEndSlash(String("\\\\")+r.substr(8));
      return TRUE;
    }
    if (r.left(4)=="\\??\\") 
    {
      res=CutEndSlash(r.substr(4));
      return TRUE;
    }
  }  

  // bugfixed by slst:
  // GetFileAttributes returns FILE_ATTRIBUTE_REPARSE_POINT
  // for drives names e.g. "E:"
  // Symlinks should have length > 2
  if (Win2K && (flg & gslExpandReparsePoints) && dir.len()>2)
  {
    int attr=GetFileAttributes(dir.ptr());
    if (attr!=0xFFFFFFFF && attr & FILE_ATTRIBUTE_DIRECTORY
      && attr & FILE_ATTRIBUTE_REPARSE_POINT)
    {
      HANDLE hf=CreateFile(dir.ptr(), 
                            GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, 
                            OPEN_EXISTING, 
                            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, 
                            NULL);
      if (hf!=INVALID_HANDLE_VALUE) 
      {
        unsigned char Data[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
        memset(Data, 0, sizeof(Data));
        DWORD returnedLength = 0;
        if (DeviceIoControl(hf, FSCTL_GET_REPARSE_POINT, NULL, 0, 
                            Data, sizeof(Data), &returnedLength, NULL)) 
        {
          REPARSE_GUID_DATA_BUFFER *rd = (REPARSE_GUID_DATA_BUFFER*)Data;
          if ((IsReparseTagMicrosoft(rd->ReparseTag)) &&
               (rd->ReparseTag==IO_REPARSE_TAG_MOUNT_POINT) && 
               (!memcmp(rd->ReparseGuid.Data4, L"\\??\\", 8)))
          {
            wchar_t buf[MAX_FILENAME];
            memset(buf, 0, sizeof(buf));
            _wtotcs(buf, (wchar_t*)&rd->GenericReparseBuffer, MAX_FILENAME);
            String r=buf;
            if (r.left(7)!="Volume{") 
            {
              CloseHandle(hf);
              res=CutEndSlash(buf);
              return TRUE;
            }
          }
        }
        CloseHandle(hf);
      }
    }
  }

  if (Win2K && (flg & gslExpandMountPoints))
  {
    // bugfixed by slst: bug #34
    // 
    //if (pGetVolumeNameForVolumeMountPoint(AddEndSlash(dir).ptr(), buf, MAX_FILENAME))
    if (GetPrimaryVolumeMountPoint(dir, res))
    {
      return TRUE;
    }
  }

  if (flg & gslExpandNetMappings && (dir.len()==2) && (dir[1]==':') && 
      WNetGetConnection(dir.ptr(), buf, &sz)==0)
  {
    res=CutEndSlash(buf);
    return TRUE;
  }

  return FALSE;
}


String GetRealFileName(const String& _path, int flg)
{
  String path=AddEndSlash(_path);
rep:
  for (int i=path.len()-1; i>=0; i--)
  {
    if (path[i]=='\\')
    {
      String res;
      if (GetSymLink(path.substr(0, i), res, 
                      gslExpandSubst | gslExpandReparsePoints | gslExpandMountPoints |
                      ((flg&rfnNoNetExpand)?0:gslExpandNetMappings))) 
      {
        path=AddEndSlash(res+path.substr(i));
        goto rep;
      }
    }
  }
  return CutEndSlash(path);
}

String GetFileRoot(const String& _path)
{
  String path=GetRealFileName(_path), res;
  if (Win2K)
  {
    wchar_t buf[MAX_FILENAME];
    if (GetVolumePathName(path.ptr(), buf, MAX_FILENAME))
      res=AddEndSlash(buf);
    else
      res=GetFileNameRoot(path);
  }
  else res=GetFileNameRoot(path);
  return res;
}

String ExpandEnv(const String& v)
{
  wchar_t buf[MAX_FILENAME];
  ExpandEnvironmentStrings(v.ptr(), buf, MAX_FILENAME);
  return buf;
}

String ApplyFileMask(const String& _name, const String& _mask)
{
  wchar_t *name=_name.Lock(), 
        *mask=_mask.Lock(),
        res[MAX_FILENAME]=_T("");
  int sz=MAX_FILENAME;
  wchar_t *next = (wchar_t*)_tcsend(name)-1,
        *mext = (wchar_t*)_tcsend(mask)-1; 
  while (next >= name && *next != '.') next--;
  if (next < name) next = name+_tcslen(name);
  else *next++ = 0;
  while (mext >= mask && *mext != '.') mext--;
  if (mext < mask) mext = mask+_tcslen(mask);
  else *mext++ = 0;
  wchar_t sym[2] = { 0, 0 };
  for (wchar_t *m = mask; *m; m++)
  {
    if (*m == '*') _tcat(res, name, sz);
    else if (*m == '?') _tcat(res, 
      (*sym = m-mask < (int)_tcslen(name) ? name[m-mask] : 0, sym), sz);
    else _tcat(res, (*sym=*m, sym), sz);
  }
  if (mext[0])
  {
    _tcat(res, _T("."), sz);
    for (wchar_t *m = mext; *m; m++)
    {
      if (*m == '*') _tcat(res, next, sz);
      else if (*m == '?') _tcat(res, 
        (*sym = m-mext < (int)_tcslen(next) ? next[m-mext] : 0, sym), sz);
      else _tcat(res, (*sym=*m, sym), sz);
    }
  }
  _name.Unlock();
  _mask.Unlock();
  return res;
}

String ApplyFileMaskPath(const String& name, const String& mask)
{
  if (mask[mask.len()-1]=='\\' || mask[mask.len()-1]=='/')
    return mask+ExtractFileName(name);
  int a = GetFileAttributes(mask.ptr());
  if (a != 0xFFFFFFFF && a & FILE_ATTRIBUTE_DIRECTORY)
  {
    String res=mask;
    if (name.icmp(mask)) res+=String("\\")+ExtractFileName(name);
    return res;
  }
  return ExtractFilePath(mask)+"\\"+
    ApplyFileMask(ExtractFileName(name), ExtractFileName(mask));
}

int FileExists(const String& name)
{
  return GetFileAttributes(name.ptr())!=0xFFFFFFFF;
}

__int64 FileSize(HANDLE h)
{
  ULONG hsz, lsz=GetFileSize(h, &hsz);
  return MAKEINT64(lsz, hsz);
}

__int64 FileSize(const String& fn)
{
  WIN32_FIND_DATA fd;
  HANDLE h = FindFirstFile(fn.ptr(), &fd);
  if (h != INVALID_HANDLE_VALUE)
  {
    FindClose(h);
    return MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh);
  }
  else 
    return 0;
}

inline String TempName()
{
  srand((unsigned)time(NULL));
  return Format("%8.8x", rand()*rand()*rand())+
    "."+Format("%3.3x", rand());
}

String TempPath()
{
  wchar_t buf[MAX_FILENAME];
  GetTempPath(MAX_FILENAME, buf);
  return CutEndSlash(buf);
}

String TempPathName()
{
  return TempPath()+"\\"+TempName();
}

static int __MoveFile(wchar_t* src, wchar_t *dst)
{
  int attr=GetFileAttributes(dst);
  SetFileAttributes(dst, FILE_ATTRIBUTE_NORMAL);
  if (::MoveFile(src, dst)) return TRUE;
  else
  {
    int err=GetLastError();
    SetFileAttributes(dst, attr);
    SetLastError(err);
    return FALSE;
  }
}

static int __MoveFileEx(wchar_t* src, wchar_t *dst, int flg)
{
  int attr=GetFileAttributes(dst);
  // bug #41 fixed by axxie
  if (_tcsicmp(src, dst)) SetFileAttributes(dst, FILE_ATTRIBUTE_NORMAL);
  if (::MoveFileEx(src, dst, flg)) return TRUE;
  else
  {
    int err=GetLastError();
    SetFileAttributes(dst, attr);
    SetLastError(err);
    return FALSE;
  }
}

int MoveFile(const String& _src, const String& _dst, int replace)
{
  // bugfixed by slst: bug #31
  // return false if dst is hard link
  if (WinNT && replace)
  {
    HANDLE DstFileHandle = Open(_dst, OPEN_READ);
    if (DstFileHandle == NULL) return FALSE;
    BY_HANDLE_FILE_INFORMATION FileInformation;
    if (GetFileInformationByHandle(DstFileHandle, &FileInformation))
    {
      Close(DstFileHandle);
      if (!(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
          (FileInformation.nNumberOfLinks > 1))
      {
        return FALSE; // Hardlinks are replaced by FileCopyEx engine
      }
    }
    else
    {
      Close(DstFileHandle);
      return FALSE; // cannot get info by handle
    }
  }
  
  ForceDirectories(_dst);
  if (WinNT)
  {
    return __MoveFileEx(_src.ptr(), _dst.ptr(), replace ? MOVEFILE_REPLACE_EXISTING: 0);
  }
  else
  {
    String root1=GetFileRoot(_src),
           root2=GetFileRoot(_dst);
    if (root1=="" || root2=="")
    {
      SetLastError(ERROR_PATH_NOT_FOUND);
      return FALSE;
    }
    if (!root1.icmp(root2))
    {
      String src=GetRealFileName(_src),
             dst=GetRealFileName(_dst);
      if (!src.icmp(dst)) return TRUE;
      int sa=GetFileAttributes(src.ptr());
      if (sa==0xFFFFFFFF)
      {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
      }
      int da=GetFileAttributes(dst.ptr());
      if (da!=0xFFFFFFFF)
      {
        if (sa & FILE_ATTRIBUTE_DIRECTORY || da & FILE_ATTRIBUTE_DIRECTORY)
        {
          SetLastError(ERROR_ACCESS_DENIED);
          return FALSE;
        }
        if (!replace)
        {
          SetLastError(ERROR_ALREADY_EXISTS);
          return FALSE;
        }
        String temp=ExtractFilePath(dst)+"\\"+TempName();
        if (!__MoveFile(dst.ptr(), temp.ptr())) return FALSE;
        if (!__MoveFile(src.ptr(), dst.ptr()))
        {
          int err=GetLastError();
          __MoveFile(temp.ptr(), dst.ptr());
          SetLastError(err);
          return FALSE;
        }
        SetFileAttributes(temp.ptr(), FILE_ATTRIBUTE_NORMAL);
        DeleteFile(temp.ptr());
        return TRUE;
      }
      return __MoveFile(src.ptr(), dst.ptr());
    }
    else
    {
      SetLastError(ERROR_NOT_SAME_DEVICE);
      return FALSE;
    }
  }
}

void ForceDirectories(const String& s)
{
  wchar_t *ptr = s.Lock(), *sptr=ptr;
  while (*ptr)
  {
    if (*ptr == '\\' || *ptr == '/')
    {
      wchar_t t = *ptr;
      *ptr = 0;
      CreateDirectory(sptr, NULL);
      *ptr = t;
    }
    ptr++;
  }
  s.Unlock();
}

void Out(const String &s)
{
  DWORD cb;
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (const void*)s.ptr(), 
    s.len(), &cb, NULL);
}