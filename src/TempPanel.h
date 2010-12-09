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

#include "../framework/far/farplugin.h"

class TempPanel : public FarPanel
{
public:
  TempPanel();
  ~TempPanel();

  int ReadFileList(int silent);
  int ChangeDir(String& dir, int silent, const String& suggest);
  int DelFiles(PluginPanelItem* files, int count, int silent);
  int PutFiles(PluginPanelItem* files, int count, int move, int silent);
  int GetFiles(PluginPanelItem* files, int count, int move, 
    const String& dest, int silent);
  int MkDir(String& name, int silent);

};
