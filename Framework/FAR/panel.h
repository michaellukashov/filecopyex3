/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010 Serge Cheperis aka craZZy
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

#pragma once

#include "interface/plugin.hpp"
#include "../array.h"
#include "../objstring.h"
#include "../fileutils.h"

class FarPanel
{
public:
  FarPanel();
  virtual ~FarPanel();

  void CallGetOpenPluginInfo(OpenPluginInfo *info);
  int CallSetDirectory(const String& dir, int opmode);
  int CallGetFindData(PluginPanelItem* &items, int &nitems, int opmode);
  void CallFreeFindData(PluginPanelItem *items, int nitems);
  int CallDeleteFiles(PluginPanelItem *items, int nitems, int opmode);
  int CallPutFiles(PluginPanelItem *items, int nitems, int move, int opmode);
  int CallGetFiles(PluginPanelItem *items, int nitems, int move, 
    const String& dest, int opmode);
  int CallMakeDirectory(String& name, int opmode);

  int Flags;
  String FormatName, PanelTitle, CurDir;
  Array<PluginPanelItem> Files;

  virtual int ChangeDir(String& dir, int silent, const String& suggest);
  virtual int ReadFileList(int silent);
  virtual int DelFiles(PluginPanelItem* files, int count, int silent);
  virtual int PutFiles(PluginPanelItem* files, int count, int move, int silent);
  virtual int GetFiles(PluginPanelItem* files, int count, int move, 
    const String& dest, int silent);
  virtual int MkDir(String& name, int silent);

private:
  wchar_t _TitleBuf[128], _FormatBuf[128], _CurDirBuf[MAX_FILENAME];
  int Rescan;
};
