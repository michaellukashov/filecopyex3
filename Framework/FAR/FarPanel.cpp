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

#include "../stdhdr.h"
#include "farplugin.h"
#include "farpanel.h"

FarPanel::FarPanel()
{
  Flags=OPIF_USEFILTER | OPIF_USESORTGROUPS | OPIF_USEHIGHLIGHTING |
    OPIF_REALNAMES | OPIF_SHOWNAMESONLY;
  CurDir="";
  FormatName="someplug";
  PanelTitle="Some plugin";
  Rescan=1;
}

FarPanel::~FarPanel()
{
}

void FarPanel::CallGetOpenPluginInfo(OpenPluginInfo *info)
{
  info->StructSize=sizeof(*info);
  info->Flags=Flags;
  info->HostFile=NULL;
  (FormatName+":\\"+CurDir).ToUnicode(_CurDirBuf, MAX_FILENAME);
  info->CurDir=_CurDirBuf;
  FormatName.ToUnicode(_FormatBuf, sizeof(_FormatBuf)/sizeof(wchar_t));
  PanelTitle.ToUnicode(_TitleBuf, sizeof(_TitleBuf)/sizeof(wchar_t));
  info->Format=_FormatBuf;
  info->PanelTitle=_TitleBuf;
}

int FarPanel::CallSetDirectory(const String& _dir, int opmode)
{
  String dir=_dir, NewDir=CurDir;
  if (dir=="..") 
  {
    if (NewDir=="") 
    {
      if (Flags & OPIF_ADDDOTS) 
        Info.Control(this, FCTL_CLOSEPLUGIN, 0, NULL);
      else 
        return FALSE;
    }
    int p=NewDir.crfind('\\');
    if (p==-1) NewDir="";
    NewDir=NewDir.substr(0, p);
  }
  else if (dir.left(FormatName.len()+2)==FormatName+":\\") 
    NewDir=NewDir.substr(FormatName.len()+2);
  else 
  {
    if (NewDir!="") NewDir+="\\";
    NewDir+=dir;
  }
  int res=ChangeDir(dir, opmode & OPM_FIND, NewDir);
  if (res) Rescan=1;
  return res;
}

int FarPanel::CallGetFindData(PluginPanelItem* &items, int &nitems, int opmode)
{
  if (Rescan)
  {
    Files.Clear();
    if (!ReadFileList(opmode & OPM_FIND)) return FALSE;
    Rescan=0;
  }
  items=(PluginPanelItem*)Files.Storage();
  nitems=Files.Count();
  return TRUE;
}

void FarPanel::CallFreeFindData(PluginPanelItem *items, int nitems)
{
  Files.Clear();
  Rescan=1;
}

int FarPanel::CallDeleteFiles(PluginPanelItem *items, int nitems, int opmode)
{
  int res=DelFiles(items, nitems, opmode & OPM_SILENT);
  Rescan=1;
  return res;
}

int FarPanel::CallPutFiles(PluginPanelItem *items, int nitems, int move, int opmode)
{
  int res=PutFiles(items, nitems, move, opmode & OPM_SILENT);
  Rescan=1;
  return res;
}

int FarPanel::CallGetFiles(PluginPanelItem *items, int nitems, int move, 
                           const String& dest, int opmode)
{
  int res=GetFiles(items, nitems, move, dest, opmode & OPM_SILENT);
  Rescan=1;
  return res;
}

int FarPanel::CallMakeDirectory(String &name, int opmode)
{
  int res=MkDir(name, opmode & OPM_SILENT);
  Rescan=1;
  return res;
}

int FarPanel::ChangeDir(String& dir, int silent, const String& suggest)
{
  CurDir=suggest;
  return TRUE;
}

int FarPanel::ReadFileList(int silent)
{
  return TRUE;
}

int FarPanel::DelFiles(PluginPanelItem* files, int count, int silent)
{
  return FALSE;
}

int FarPanel::PutFiles(PluginPanelItem* files, int count, int move, int silent)
{
  return FALSE;
}

int FarPanel::GetFiles(PluginPanelItem* files, int count, int move, 
                       const String& dest, int silent)
{
  return FALSE;
}

int FarPanel::MkDir(String &name, int silent)
{
  return FALSE;
}

void _export WINAPI GetOpenPluginInfoW(HANDLE hPlugin, struct OpenPluginInfo *Info)
{
//  SetFileApisToANSI();
  ((FarPanel*)hPlugin)->CallGetOpenPluginInfo(Info);
//  SetFileApisToOEM();
}

int _export WINAPI SetDirectoryW(HANDLE hPlugin,const wchar_t *Dir,int OpMode)
{
//  SetFileApisToANSI();
//   size_t l = wcslen(Dir)+1;
//   char *dir=(char*)_alloca(l);
//   strcpy_s(dir, l, Dir);
//   _toansi(dir);
  int res=((FarPanel*)hPlugin)->CallSetDirectory(Dir, OpMode);
//  SetFileApisToOEM();
  return res;
}

int _export WINAPI GetFindDataW(HANDLE hPlugin,  struct PluginPanelItem **pPanelItem,
                               int *pItemsNumber, int OpMode)
{
//  SetFileApisToANSI();
  int res=((FarPanel*)hPlugin)->CallGetFindData(*pPanelItem, *pItemsNumber, OpMode);
//  SetFileApisToOEM();
  return res;
}

void _export WINAPI FreeFindDataW(HANDLE hPlugin, struct PluginPanelItem *PanelItem,
                                 int ItemsNumber)
{
//  SetFileApisToANSI();
  ((FarPanel*)hPlugin)->CallFreeFindData(PanelItem, ItemsNumber);
//  SetFileApisToOEM();
}

int _export WINAPI DeleteFilesW(HANDLE hPlugin, struct PluginPanelItem *PanelItem,
                               int ItemsNumber, int OpMode)
{
//  SetFileApisToANSI();
  int res=((FarPanel*)hPlugin)->CallDeleteFiles(PanelItem, ItemsNumber, OpMode);
//  SetFileApisToOEM();
  return res;
}

int _export WINAPI PutFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t *SrcPath,int OpMode)
{
//  SetFileApisToANSI();
  int res=((FarPanel*)hPlugin)->CallPutFiles(PanelItem, ItemsNumber, Move, OpMode);
//  SetFileApisToOEM();
  return res;
}

int _export WINAPI GetFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t **DestPath,int OpMode)
{
//  SetFileApisToANSI();
//   size_t l = strlen(DestPath)+1;
//   char *dp=(char*)_alloca(l);
//   strcpy_s(dp, l, DestPath);
//   _toansi(dp);
  int res=((FarPanel*)hPlugin)->CallGetFiles(PanelItem, ItemsNumber, Move, *DestPath, OpMode);
//  SetFileApisToOEM();
  return res;
}

int _export WINAPI MakeDirectoryW(HANDLE hPlugin,const wchar_t **Name,int OpMode)
{
//  SetFileApisToANSI();
//   size_t l = strlen(Name)+1;
//   char *dp=(char*)_alloca(l);
//   strcpy_s(dp, l, Name);
//   _toansi(dp);
  String name=*Name;
  int res=((FarPanel*)hPlugin)->CallMakeDirectory(name, OpMode);
//   if (res==TRUE && name!=Name)
// 	  name.ToOem(Name, MAX_PATH);
//  SetFileApisToOEM();
  return res;

}
