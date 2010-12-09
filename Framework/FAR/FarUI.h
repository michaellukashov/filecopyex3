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
#include "../array.h"
#include "interface/plugin.hpp"

class FarMenu
{
public:
  FarMenu(void);
  virtual ~FarMenu(void);
  void SetTitle(const String&);
  void SetBottom(const String&);
  void SetHelpTopic(const String&);
  void SetFlags(int f);
  void AddLine(const String&);
  void AddLineCheck(const String&, int check);
  void AddSep();
  void SetSelection(int n);
  int Execute();
protected:
	void SetItemText(FarMenuItem* item, const String& text);
  String Title, Bottom, HelpTopic;
  int Flags, Selection;
  Array<FarMenuItem> items;
};

int ShowMessage(const String&, const String&, int);
int ShowMessageHelp(const String&, const String&, int, const String&);
int ShowMessageEx(const String&, const String&, const String&, int);
int ShowMessageExHelp(const String&, const String&, const String&, int, const String&);

#define RES_RETRY 1
#define RES_SKIP 0

void Error(const String&, int code);
void Error2(const String&, const String&, int code);
int Error2RS(const String&, const String&, int code);

String GetErrText(int code);

String FormatWidth(const String&, int);
String FormatWidthNoExt(const String&, int);
String SplitWidth(const String&, int);

inline __int64 GetTime()
{
  LARGE_INTEGER res;
  QueryPerformanceCounter(&res);
  return res.QuadPart;
}

inline __int64 TicksPerSec()
{
  LARGE_INTEGER res;
  QueryPerformanceFrequency(&res);
  return res.QuadPart;
}

