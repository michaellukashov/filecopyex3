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
#include "temppanel.h"
#include "../framework/strutils.h"
#include "filecopyex.h"
#include "common.h"
#include "../framework/common.h"

StringList TempFiles;

TempPanel::TempPanel()
{
  Flags |= OPIF_ADDDOTS | OPIF_REALNAMES | OPIF_EXTERNALGET;
  PanelTitle=" Copy queue ";
  FormatName="queue";
//  TempFiles.SetOptions(slIgnoreCase);
}

TempPanel::~TempPanel()
{
}

void CopyFindData(const String& _fn, WIN32_FIND_DATA fd, PluginPanelItem& item,
                  int toplevel)
{
  String fn=_fn;
  memset(&item, 0, sizeof(item));
  if(!toplevel)
  {
    String name = fd.cFileName;
    fn = ExtractFilePath(fn)+ "\\" + name;
  }
  size_t len = (fn.len() + 1)*sizeof(wchar_t);
  wchar_t* name = (wchar_t*)malloc(len);
  fn.ToUnicode(name, fn.len() + 1);
  item.FindData.lpwszFileName = name;
  item.FindData.lpwszAlternateFileName = NULL;
  item.FindData.dwFileAttributes=fd.dwFileAttributes;
  item.FindData.ftCreationTime=fd.ftCreationTime;
  item.FindData.ftLastAccessTime=fd.ftLastAccessTime;
  item.FindData.ftLastWriteTime=fd.ftLastWriteTime;
  item.FindData.nFileSize = MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh);
//   if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//     item.PackSize=item.PackSizeHigh=0;
//   else
//     item.PackSize=GetCompressedFileSize(fn.ptr(), &item.PackSizeHigh);
}

int TempPanel::ReadFileList(int silent)
{
  if (CurDir=="")
  {
    int i=0, changed=0;
    while (i<TempFiles.Count())
    {
      WIN32_FIND_DATA fd;
      String fn=TempFiles[i];
      HANDLE hf=FindFirstFile(fn.ptr(), &fd);
      if (hf==INVALID_HANDLE_VALUE)
      {
        TempFiles.Delete(i);
        changed=1;
        continue;
      }
      FindClose(hf);
      PluginPanelItem item;
      CopyFindData(fn, fd, item, 1);
      Files.Add(item);
      i++;
    }
    if (changed) SaveTemp();
    return TRUE;
  }
  else
  {
    String path=AddEndSlash(Replace(CurDir, "/", "\\"));
    WIN32_FIND_DATA fd;
    HANDLE hf=FindFirstFile((path+"*.*").ptr(), &fd);
    if (hf==INVALID_HANDLE_VALUE) return FALSE;
    do
    {
      if (_tcscmp(fd.cFileName, _T(".")) && _tcscmp(fd.cFileName, _T("..")))
      {
        PluginPanelItem item;
        CopyFindData(path+fd.cFileName, fd, item, 0);
        Files.Add(item);
      }
    }
    while (FindNextFile(hf, &fd));
    FindClose(hf);
    return TRUE;
  }
}

int TempPanel::ChangeDir(String& dir, int silent, const String& suggest)
{
  if (dir==".." || dir=="\\") CurDir=suggest;
  else if (CurDir=="") CurDir=Replace(suggest, "\\", "/");
  else CurDir+=String("\\")+ExtractFileName(suggest);

  if (CurDir!="") 
  {
    String rd=Replace(CurDir, "/", "\\");
    PanelTitle=String(" Queue: ")+rd+" ";
    Flags |= OPIF_EXTERNALPUT | OPIF_EXTERNALDELETE 
      | OPIF_EXTERNALMKDIR;
  }
  else
  {
    PanelTitle=" Copy queue ";
    Flags &= ~(OPIF_EXTERNALPUT | OPIF_EXTERNALDELETE
      | OPIF_EXTERNALMKDIR);
  }
  return TRUE;
}

void CallCopy(int move, int curonly);

int TempPanel::DelFiles(PluginPanelItem* files, int count, int silent)
{
  if (CurDir=="")
  {
    wchar_t buf[MAX_FILENAME];
    for (int i=0; i<count; i++)
    {
      wcscpy_s(buf, MAX_FILENAME, files[i].FindData.lpwszFileName);
      int j=TempFiles.Find(buf);
      if (j!=-1) TempFiles.Delete(j);
    }
    SaveTemp();
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int TempPanel::PutFiles(PluginPanelItem* files, int count, int move, int silent)
{
  if (CurDir=="")
  {
    wchar_t buf[MAX_FILENAME];
    GetCurrentDirectory(MAX_FILENAME, buf);
    String cd=AddEndSlash(buf);
    for (int i=0; i<count; i++)
    {
      String file=files[i].FindData.lpwszFileName;
      if (file.cfind('\\')==-1) file=cd+file;
      if (TempFiles.Find(file)==-1) TempFiles.Add(file);
    }
    SaveTemp();
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int TempPanel::GetFiles(PluginPanelItem* files, int count, int move, 
                        const String& dest, int silent)
{
  return FALSE;
}

int TempPanel::MkDir(String &name, int silent)
{
  return TRUE;
}

