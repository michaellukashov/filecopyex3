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

#include "../valuelist.h"
#include "../stringlist.h"
#include "interface/plugin.hpp"
#include "interface/farcolor.hpp"
#include "fardialog.h"
#include "farregistry.h"
#include "farui.h"
#include "farprogress.h"
#include "farpanel.h"

class FarPlugin
{
public:
  FarPlugin(void);
  virtual ~FarPlugin(void);
  virtual int Configure(int);
  virtual FarPanel* OpenPlugin(int, int);
  StringList MenuItems, ConfigItems;
  String Prefix, RootKey;
  int Flags;
  FarDialogList Dialogs;
  void InitLang();
  PropertyList Options;
  virtual void LoadOptions();
  virtual void SaveOptions();
  virtual void InitOptions(PropertyList&);
};

extern LocaleList* Locale;
#define LOC(s) ((*Locale)[s])

extern PluginStartupInfo Info;
extern String PluginRootKey;
extern FarPlugin *Instance;
extern HANDLE hInstance;

String GetDLLName();
String GetDLLPath();

FarPlugin* InitInstance();

void FarErrorHandler(const wchar_t*);
