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


#include "Framework/StrUtils.h"
#include "SDK/farcolor.hpp"
#include "FarProgress.h"
#include "Common.h"
#include "ui.h"
#include "tools.h"
#include "guid.hpp"

FarProgress::FarProgress(void)
{
  Info.AdvControl(&MainGuid, ACTL_GETCOLOR, COL_DIALOGBOX, &clrFrame);
  Info.AdvControl(&MainGuid, ACTL_GETCOLOR, COL_DIALOGBOXTITLE, &clrTitle);
  Info.AdvControl(&MainGuid, ACTL_GETCOLOR, COL_DIALOGTEXT, &clrBar);
  Info.AdvControl(&MainGuid, ACTL_GETCOLOR, COL_DIALOGTEXT, &clrText);
  /* switch (clrFrame >> 4)
  {
    // [exp] color changed by CDK
    case 7: case 15: clrLabel=1; break;
    case 10: case 11: clrLabel=15; break;
    default: clrLabel=14; break;
  }
  clrLabel=clrLabel | clrFrame & 0xF0;
  */
  Info.AdvControl(&MainGuid, ACTL_GETCOLOR, COL_DIALOGTEXT, &clrLabel); // !!! not sure

  ProgX1 = 0;
  ProgX2 = 0;
  ProgY = 0;
  WinType = WIN_NONE;
  hScreen = 0;
  InverseBars = 0;
  LastUpdate = 0;
  NeedToRedraw = false;
}

FarProgress::~FarProgress(void)
{
  Hide();
}

void FarProgress::DrawWindow(int X1, int Y1, int X2, int Y2, const String & caption)
{
  int W = X2 - X1 + 1, H = Y2 - Y1 + 1;
  String tpl = caption;
  tpl += L"\n";
  String bkg = String(' ', W - 10);
  bkg += L"\n";
  for (int Index = 0; Index < H - 4; ++Index)
  {
    tpl += bkg;
  }
  Info.Message(&MainGuid, &ProgressDlg, FMSG_LEFTALIGN | FMSG_ALLINONE, nullptr,
               reinterpret_cast<const wchar_t * const *>(tpl.ptr()), 0, 0);
}

void FarProgress::GetConSize(int & w, int & h)
{
  HANDLE hc = ::GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO bi;
  ::GetConsoleScreenBufferInfo(hc, &bi);
  w = bi.srWindow.Right - bi.srWindow.Left + 1;
  h = bi.srWindow.Bottom - bi.srWindow.Top + 1;
}

void FarProgress::ShowMessage(const String & msg)
{
  Hide();
  int sw, sh;
  GetConSize(sw, sh);
  int W = (int)msg.len() + 12, H = 5;
  int X1 = (sw - W + 1) / 2;
  int Y1 = (sh - H - 1) / 2;
  int X2 = X1 + W - 1, Y2 = Y1 + H - 1;
  hScreen = Info.SaveScreen(X1, Y1, X2 + 2, Y2 + 2);
  DrawWindow(X1, Y1, X2, Y2, L"");
  Info.Text(X1 + 6, Y1 + 2, &clrText, FormatWidth(msg, X2 - X1 - 11).ptr());
  Info.Text(0, 0, 0, nullptr);
  WinType = WIN_MESSAGE;
  TitleBuf = GetTitle();
  SetTitle2(msg);
  Info.RestoreScreen(nullptr);
}

void FarProgress::ShowProgress(const String & msg)
{
  Hide();
  int sw, sh;
  GetConSize(sw, sh);
  int W = sw / 2, H = 6;
  int X1 = (sw - W + 1) / 2;
  int Y1 = (sh - H - 1) / 2;
  int X2 = X1 + W - 1, Y2 = Y1 + H - 1;
  hScreen = Info.SaveScreen(X1, Y1, X2 + 2, Y2 + 2);
  DrawWindow(X1, Y1, X2, Y2, L"");
  Info.Text(X1 + 5, Y1 + 2, &clrText, FormatWidth(msg, X2 - X1 - 9).ptr());
  ProgX1 = X1 + 5;
  ProgX2 = X2 - 5;
  ProgY = Y1 + 3;
  WinType = WIN_PROGRESS;
  SetPercent(0);
  DrawProgress(ProgX1, ProgX2, ProgY, 0);
  Info.Text(0, 0, 0, nullptr);
  TitleBuf = GetTitle();
  ProgTitle = msg;
  SetTitle2(msg);
  Info.RestoreScreen(nullptr);
}

void FarProgress::DrawProgress(int x1, int x2, int y, float pc)
{
  int n = x2 - x1 + 1;
  int fn = (int)(pc * n);
  int en = n - fn;
  wchar_t buf[512];
  wchar_t * bp = buf;
  if (!InverseBars)
  {
    for (int Index = 0; Index < fn; Index++)
      *bp++ = 0x2588; //'█'
    for (int Index = 0; Index < en; Index++)
      *bp++ = 0x2591; //'░'
  }
  else
  {
    for (int Index = 0; Index < en; Index++)
      *bp++ = 0x2591; //'░'
    for (int Index = 0; Index < fn; Index++)
      *bp++ = 0x2588; //'█'
  }
  *bp = 0;
  Info.Text(x1, y, &clrText, buf);
  taskbarIcon.SetState(taskbarIcon.S_PROGRESS, pc);
}

void FarProgress::SetPercent(float pc)
{
  if (WinType == WIN_PROGRESS)
  {
    if (GetTime() - LastUpdate > TicksPerSec() / 5)
    {
      DrawProgress(ProgX1, ProgX2, ProgY, pc);
      Info.Text(0, 0, 0, nullptr);
      SetTitle2(ProgTitle + L" {" + String((int)(pc * 100)) + L"%}");
      LastUpdate = GetTime();
    }
  }
}

void FarProgress::SetNeedToRedraw(bool Value)
{
  NeedToRedraw = Value;
}

void FarProgress::Hide()
{
  if (WinType != WIN_NONE)
  {
    Info.RestoreScreen(nullptr);
    Info.RestoreScreen(hScreen);
    SetTitle(TitleBuf);
    hScreen = 0;
  }
  WinType = WIN_NONE;
  taskbarIcon.SetState(taskbarIcon.S_NO_PROGRESS);
}

void FarProgress::Text(intptr_t x, intptr_t y, FarColor * c, const String & msg)
{
  Info.Text(x, y, c, msg.ptr());
}

void FarProgress::SetTitle(const String & v)
{
  SetConsoleTitle(v.ptr());
}

void FarProgress::SetTitle2(const String & v) const
{
  String far_desc = TitleBuf;
  size_t x = far_desc.find('-');
  if (x != (size_t)-1)
    far_desc = far_desc.substr(x);
  else
    far_desc = L"- Far";
  SetTitle(v + L" " + far_desc);
}

String FarProgress::GetTitle()
{
  wchar_t buf[512];
  GetConsoleTitle(buf, LENOF(buf));
  return buf;
}


// New class member function
void FarProgress::ShowScanProgress(const String & msg)
{
  Hide();
  int ConsoleWidth;
  int ConsoleHeight;
  GetConSize(ConsoleWidth, ConsoleHeight);
  int WindowWidth = ConsoleWidth / 2;
  if (WindowWidth > 50)
    WindowWidth = 50;
  if (WindowWidth < 40)
    WindowWidth = 40;
  int WindowHeight = 7;
  int WindowCoordX1 = (ConsoleWidth  - WindowWidth + 1) / 2;
  int WindowCoordY1 = (ConsoleHeight - WindowHeight - 1) / 2;
  int WindowCoordX2 = WindowCoordX1 + WindowWidth - 1;
  int WindowCoordY2 = WindowCoordY1 + WindowHeight - 1;
  hScreen = Info.SaveScreen(WindowCoordX1, WindowCoordY1,
                            WindowCoordX2 + 2, WindowCoordY2 + 2);
  DrawWindow(WindowCoordX1, WindowCoordY1, WindowCoordX2, WindowCoordY2, L"");
  Info.Text(WindowCoordX1 + 5, WindowCoordY1 + 2, &clrText,
            FormatWidth(msg, WindowCoordX2 - WindowCoordX1 - 9).ptr());
  ProgX1 = WindowCoordX1 + 5;
  ProgX2 = WindowCoordX2 - 5;
  ProgY  = WindowCoordY1 + 3;
  WinType = WIN_SCAN_PROGRESS;
  DrawScanProgress(ProgX1, ProgX2, ProgY, 0, 0);
  Info.Text(0, 0, 0, nullptr);
  TitleBuf = GetTitle();
  ProgTitle = msg;
  SetTitle2(msg);
  Info.RestoreScreen(nullptr);
}

// New class member function
void FarProgress::SetScanProgressInfo(int64_t NumberOfFiles, int64_t TotalSize)
{
  if (WinType == WIN_SCAN_PROGRESS)
  {
    if (GetTime() - LastUpdate > TicksPerSec() / 5)
    {
      DrawScanProgress(ProgX1, ProgX2, ProgY, NumberOfFiles, TotalSize);
      Info.Text(0, 0, 0, nullptr);
      //SetTitle2(ProgTitle+" {"+String((int)(pc*100))+"%}");
      LastUpdate = GetTime();
    }
  }
}

// New class member function
void FarProgress::DrawScanProgress(int x1, int x2, int y, int64_t NumberOfFiles, int64_t TotalSize)
{
  String FilesFmtStr = LOC(L"Status.FilesString") + L" %-6I64d";
  wchar_t FilesStr[256];
  _snwprintf_s(FilesStr, LENOF(FilesStr), LENOF(FilesStr), (const wchar_t *)FilesFmtStr.ptr(), NumberOfFiles);

  String SizeFmtStr = LOC(L"Status.SizeString") + L" %s";
  wchar_t SizeStr[256];
  _snwprintf_s(SizeStr, LENOF(SizeStr), LENOF(SizeStr), (const wchar_t *)SizeFmtStr.ptr(), (const wchar_t *)FormatValue(TotalSize).ptr());


  int s = x2 - x1 - (int)wcslen(SizeStr) - (int)wcslen(FilesStr);
  String spacer;
  if (s > 0)
  {
    spacer = String(' ', s);
  }
  else
  {
    SetNeedToRedraw(true);
  }

  wchar_t buf[256];
  _snwprintf_s(buf, LENOF(buf), LENOF(buf), L"%s %s%s", FilesStr, spacer.ptr(), SizeStr);

  if ((int)wcslen(buf) > x2 - x1 + 2)
  {
    SetNeedToRedraw(true);
  }
  RedrawWindowIfNeeded();

  Info.Text(x1, y + 1, &clrText, buf);
  taskbarIcon.SetState(taskbarIcon.S_WORKING);
}

void FarProgress::RedrawWindowIfNeeded()
{
  if (NeedToRedraw)
  {
    NeedToRedraw = false;
    RedrawWindow();
  }
}

void FarProgress::RedrawWindow()
{
  ShowScanProgress(ProgTitle);
}
