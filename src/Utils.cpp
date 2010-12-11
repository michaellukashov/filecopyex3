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

#include "../framework/stdhdr.h"
#include "../framework/lowlevelstr.h"
#include "common.h"
#include "../framework/fileutils.h"
#include "../framework/common.h"


// axxie: special HDD ID inmplementation for NT4
#define HARDDISK_CONST_PART         _T("\\Device\\Harddisk")
#define HARDDISK_CONST_PART_LEN     (sizeof(HARDDISK_CONST_PART)/sizeof(wchar_t)-1)

int GetDriveId(const String& path, String& res)
{
  wchar_t buf[MAX_FILENAME];
  if (WinNT4)
  {
     // axxie: special HDD ID inmplementation for NT4
     if (QueryDosDevice(path.left(2).ptr(), buf, MAX_FILENAME)>0)
     {
       String s = buf;
       String tmp = "\\Device\\Harddisk";
       if (!s.nicmp(tmp, tmp.len()))
       {
         res=s.substr(tmp.len(), 1);
         return TRUE;
       }
     }
  } 
  else if (QueryDosDevice(path.left(2).ptr(), buf, MAX_FILENAME)>0)
  {
    String s=buf;
    String tmp="\\Device\\HarddiskDmVolumes\\";
    if (!s.nicmp(tmp, tmp.len()))
    {
      s=s.substr(tmp.len());
      res=s.left(s.cfind('\\'));
      return TRUE;
    }
  }
  return FALSE;
}

int GetPhysDrive(const String& _path, int& res)
{
  String path=_path;
  // bugfixed by slst:
  // if (WinXP)
  if (Win2K || WinXP) // this also works OK under w2k/sp4
  {
    if (path.left(11)!="\\\\?\\Volume{")
    {
      wchar_t buf[MAX_FILENAME];
      if (!GetVolumeNameForVolumeMountPoint(path.ptr(), buf, MAX_FILENAME)) 
        return FALSE;
      path=buf;
    }
    HANDLE hVolume=CreateFile(CutEndSlash(path).ptr(), 0, 
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
      0, NULL);
    if (hVolume==INVALID_HANDLE_VALUE) return FALSE;

    char outbuf[1024];
    DWORD ret;
    if (DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0,
      outbuf, 1024, &ret, NULL))
    {
      VOLUME_DISK_EXTENTS *ext=(VOLUME_DISK_EXTENTS*)outbuf;
      res=ext->Extents[0].DiskNumber;
      CloseHandle(hVolume);
      return TRUE;
    }
    CloseHandle(hVolume);
  }

  return FALSE;
}

#define FILE_READ_ONLY_VOLUME 0x00080000
#define FILE_NAMED_STREAMS    0x00040000

int VolFlags(const String& _path)
{
  String path=_path;
  int sl=path.crfind("\\/"),
      ml=path.cfind("*?");
  if (ml!=-1 && ml<sl) return -1;
  if (path.cfind("|<>")!=-1) return -1;
  int cl=path.cfind(':');
  if (cl!=-1 && (cl!=1 || cl!=path.crfind(':') || 
    (cl!=path.len()-1 && path.cfind("\\/")!=2))) return -1;
  if (ml!=-1) path=ExtractFilePath(path);
  String root=GetFileRoot(path);

  int res=0;
  DWORD clen, flg;
  wchar_t sysname[32];
  if (GetVolumeInformation(root.ptr(), NULL, 0, NULL, &clen, &flg, sysname, 32))
  {
    if (flg & FILE_FILE_COMPRESSION) res|=VF_COMPRESSION;
    if (flg & FILE_SUPPORTS_ENCRYPTION) res|=VF_ENCRYPTION;
    if (flg & FILE_PERSISTENT_ACLS) res|=VF_RIGHTS;
    if (flg & FILE_NAMED_STREAMS) res|=VF_STREAMS;
    // NT4 supports streams, but has no special flag for this fact
    // bug #13 fixed by Alter
    if (!_tcsicmp(sysname, TEXT("NTFS"))) res|=VF_STREAMS;
    if (flg & FILE_READ_ONLY_VOLUME) res|=VF_READONLY;
    if (flg & FILE_UNICODE_ON_DISK) res|=VF_UNICODE;
    if (GetDriveType(root.ptr())==DRIVE_CDROM) res|=VF_CDROM;
    return res;
  }
  return -1;
}

int CheckParallel(const String& _srcpath, const String& _dstpath)
{
  String root1=GetFileRoot(_srcpath),
         root2=GetFileRoot(_dstpath);
  int srctype=GetDriveType(root1.ptr()),
      dsttype=GetDriveType(root2.ptr());
  if (srctype!=dsttype) return TRUE;
  if (!root1.icmp(root2)) return FALSE;

  if (srctype == DRIVE_REMOTE) 
    return FALSE;
  else if (srctype == DRIVE_CDROM || srctype == DRIVE_REMOVABLE) 
    return TRUE;
  else if (srctype == DRIVE_FIXED)
  {
    int drv1, drv2;
    if (GetPhysDrive(root1, drv1)
      && GetPhysDrive(root2, drv2))
    {
      if (drv1 != drv2) return TRUE;
      else return FALSE;
    }
    String id1, id2;
    if (GetDriveId(root1, id1)
      && GetDriveId(root2, id2))
    {
      if (id1.icmp(id2)) return TRUE;
      else return FALSE;
    }
  }

  return -1;
}

String DupName(const String& src, int n)
{
  return ChangeFileExt(src, "")+"_"+String(n)+ExtractFileExt(src);
}

int ExistsN(const String& fn, int n)
{
  if (!n)
    return FileExists(fn);
  else
    return FileExists(DupName(fn, n));
}

int Newer(FILETIME &ft1, FILETIME &ft2)
{
  return CompareFileTime(&ft1, &ft2)>=0;
}

int Newer2(const String& fn1, FILETIME &ft2)
{
  WIN32_FIND_DATA fd;
  HANDLE hf = FindFirstFile(fn1.ptr(), &fd);
  if (hf != INVALID_HANDLE_VALUE)
  {
    FindClose(hf);
    return Newer(fd.ftLastWriteTime, ft2);
  }
  else 
    return FALSE;
}

int RmDir(const String& fn)
{
  int attr=GetFileAttributes(fn.ptr());
  SetFileAttributes(fn.ptr(), FILE_ATTRIBUTE_NORMAL);
  if (!RemoveDirectory(fn.ptr()))
  {
    SetFileAttributes(fn.ptr(), attr);
    return FALSE;
  }
  return TRUE;
}

int Delete(const String& fn)
{
  int attr=GetFileAttributes(fn.ptr());
  SetFileAttributes(fn.ptr(), FILE_ATTRIBUTE_NORMAL);
  if (!DeleteFile(fn.ptr()))
  {
    SetFileAttributes(fn.ptr(), attr);
      return FALSE;
  }
  return TRUE;
}

void DebugLog(const wchar_t *DebugMsg, ...)
{
  wchar_t MsgBuf[0x400];
  va_list ArgPtr;
  va_start(ArgPtr, DebugMsg);
  _vsntprintf_s(MsgBuf, 0x400, sizeof(MsgBuf), DebugMsg, ArgPtr);
  va_end(ArgPtr);
  OutputDebugString(MsgBuf);
}
