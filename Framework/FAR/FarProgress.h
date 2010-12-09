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

#pragma once

#include "farplugin.h"

#define WIN_NONE          0
#define WIN_MESSAGE       1
#define WIN_PROGRESS      2
#define WIN_SCAN_PROGRESS 3

class FarProgress
{
public:
  FarProgress(void);
  virtual ~FarProgress(void);
  void ShowMessage(const String&);
  void ShowProgress(const String&);
  void ShowScanProgress(const String& msg);
  void SetScanProgressInfo(__int64 NumberOfFiles, __int64 TotalSize);
  void Hide();
  void SetPercent(float);
  int InverseBars;
protected:
  int clrFrame, clrTitle, clrBar, clrText, clrLabel;
  int ProgX1, ProgX2, ProgY, WinType;
  HANDLE hScreen;
  void DrawWindow(int, int, int, int, const String&);
  void GetConSize(int&, int&);
  void DrawProgress(int, int, int, float);
  void DrawText(int, int, int, const String&);
  void SetTitle(const String&);
  void SetTitle2(const String&);
  String GetTitle();
  String TitleBuf, ProgTitle;
  void DrawScanProgress(int x1, int x2, int y, __int64 NumberOfFiles, __int64 TotalSize);
  __int64 LastUpdate;
};
