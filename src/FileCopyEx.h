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

#include "../framework/properties.h"
#include "../framework/far/farplugin.h"

#define MRES_NONE         0
#define MRES_STDCOPY      1
#define MRES_STDCOPY_RET  2

extern int Win2K, WinNT;
extern StringList DescFiles;

class FileCopyExPlugin :
  public FarPlugin
{
public:
  FileCopyExPlugin(void);
  virtual ~FileCopyExPlugin(void);
  int Configure(int);
  FarPanel* OpenPlugin(int, int);
  void InitOptions(PropertyList&);
  void Config();

private:
  void About();
  void KeyConfig();
  void MiscInit();
};

void SaveTemp();
extern StringList TempFiles;
