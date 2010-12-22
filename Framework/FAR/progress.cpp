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
#include "../../src/common.h"
#include "../lowlevelstr.h"
#include "../strutils.h"
#include "progress.h"

FarProgress::FarProgress(void)
{
  clrFrame=(int)Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void*)COL_DIALOGBOX);
  clrTitle=(int)Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void*)COL_DIALOGBOXTITLE);
  clrBar=(int)Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR,   (void*)COL_DIALOGTEXT);
  clrText=(int)Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR,  (void*)COL_DIALOGTEXT);
  switch (clrFrame >> 4)
  {
    // [exp] color changed by CDK
    case 7: case 15: clrLabel=1; break;
    case 10: case 11: clrLabel=15; break;
    default: clrLabel=14; break;
  }
  clrLabel=clrLabel | clrFrame & 0xF0;
  WinType=WIN_NONE;
  hScreen=0;
  InverseBars=0;
  LastUpdate = 0;
}

FarProgress::~FarProgress(void)
{
  Hide();
}

void FarProgress::DrawWindow(int X1, int Y1, int X2, int Y2, const String& caption)
{
	int W = X2-X1+1, H = Y2-Y1+1;
	String tpl = caption;
	tpl += "\n";
	String bkg = String(' ', W - 10);
	bkg += "\n";
	for(int i = 0; i < H - 4; ++i)
	{
		tpl += bkg;
	}
	Info.Message(Info.ModuleNumber, FMSG_LEFTALIGN|FMSG_ALLINONE, NULL, (const wchar_t**)tpl.ptr(), 0, 0);
}

void FarProgress::GetConSize(int& w, int &h)
{
  HANDLE hc = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO bi;
  GetConsoleScreenBufferInfo(hc, &bi);
  w=bi.srWindow.Right - bi.srWindow.Left + 1;
  h=bi.srWindow.Bottom - bi.srWindow.Top + 1;
}

void FarProgress::ShowMessage(const String& msg)
{
  Hide();
  int sw, sh;
  GetConSize(sw, sh);
  int W=msg.len()+12, H=5;
  int X1=(sw-W + 1)/2; 
  int Y1=(sh-H - 1)/2;
  int X2=X1+W-1, Y2=Y1+H-1;
  hScreen=Info.SaveScreen(X1, Y1, X2+2, Y2+2);
  DrawWindow(X1, Y1, X2, Y2, "");
  Info.Text(X1+6, Y1+2, clrText, FormatWidth(msg, X2-X1-11).ptr());
  Info.Text(0, 0, 0, NULL);
  WinType=WIN_MESSAGE;
  TitleBuf=GetTitle();
  SetTitle2(msg);
  Info.RestoreScreen(NULL);
}

void FarProgress::ShowProgress(const String& msg)
{
  Hide();
  int sw, sh;
  GetConSize(sw, sh);
  int W=sw/2, H=6;
  int X1=(sw-W + 1)/2; 
  int Y1=(sh-H - 1)/2;
  int X2=X1+W-1, Y2=Y1+H-1;
  hScreen=Info.SaveScreen(X1, Y1, X2+2, Y2+2);
  DrawWindow(X1, Y1, X2, Y2, "");
  Info.Text(X1+5, Y1+2, clrText, FormatWidth(msg, X2-X1-9).ptr());
  ProgX1=X1+5; 
  ProgX2=X2-5;
  ProgY=Y1+3;
  WinType=WIN_PROGRESS;
  SetPercent(0);
  DrawProgress(ProgX1, ProgX2, ProgY, 0);
  Info.Text(0, 0, 0, NULL);
  TitleBuf=GetTitle();
  ProgTitle=msg;
  SetTitle2(msg);
  Info.RestoreScreen(NULL);
}

void FarProgress::DrawProgress(int x1, int x2, int y, float pc)
{
  int n=x2-x1+1, fn=(int)(pc*n), en=n-fn;
  wchar_t buf[512], *bp=buf;
  if (!InverseBars)
  {
    for (int i=0; i<fn; i++) *bp++=0x2588;//'█'
    for (int i=0; i<en; i++) *bp++=0x2591;//'░'
  }
  else
  {
    for (int i=0; i<en; i++) *bp++=0x2591;//'░'
    for (int i=0; i<fn; i++) *bp++=0x2588;//'█'
  }
  *bp=0;
  Info.Text(x1, y, clrText, buf);
  taskbar_icon.SetState(taskbar_icon.S_PROGRESS, pc);
}

void FarProgress::SetPercent(float pc)
{
  if (WinType==WIN_PROGRESS)
  {
    if (GetTime()-LastUpdate>TicksPerSec()/5)
    {
      DrawProgress(ProgX1, ProgX2, ProgY, pc);
      Info.Text(0, 0, 0, NULL);
      SetTitle2(ProgTitle+" {"+String((int)(pc*100))+"%}");
      LastUpdate=GetTime();
    }
  }
}

void FarProgress::Hide()
{
  if (WinType!=WIN_NONE) 
  {
    Info.RestoreScreen(NULL);
    Info.RestoreScreen(hScreen);
    SetTitle(TitleBuf);
    hScreen=0;
  }
  WinType=WIN_NONE;
  taskbar_icon.SetState(taskbar_icon.S_NO_PROGRESS);
}

void FarProgress::DrawText(int x, int y, int c, const String& msg)
{
  Info.Text(x, y, c, msg.ptr());
}

void FarProgress::SetTitle(const String& v)
{
  SetConsoleTitle(v.ptr());
}

void FarProgress::SetTitle2(const String& v)
{
	String far_desc = TitleBuf;
	int x = far_desc.cfind(L'-');
	if(x != -1)
		far_desc = far_desc.substr(x);
	else
		far_desc = L"- Far";
	SetTitle(v + " " + far_desc);
}

String FarProgress::GetTitle()
{
  wchar_t buf[512];
  GetConsoleTitle(buf, 512);
  return buf;
}


// bugfixed by slst: bug #24
// New class member function
void FarProgress::ShowScanProgress(const String& msg)
{
  Hide();
  int ConsoleWidth;
  int ConsoleHeight;
  GetConSize(ConsoleWidth, ConsoleHeight);
  int WindowWidth  = ConsoleWidth / 2;
  if (WindowWidth > 46) WindowWidth = 46;
  if (WindowWidth < 40) WindowWidth = 40;
  int WindowHeight = 7;
  int WindowCoordX1 = (ConsoleWidth  - WindowWidth + 1)/2;
  int WindowCoordY1 = (ConsoleHeight - WindowHeight - 1)/2;
  int WindowCoordX2 = WindowCoordX1 + WindowWidth - 1;
  int WindowCoordY2 = WindowCoordY1 + WindowHeight - 1;
  hScreen = Info.SaveScreen(WindowCoordX1, WindowCoordY1, 
                            WindowCoordX2 + 2, WindowCoordY2 + 2);
  DrawWindow(WindowCoordX1, WindowCoordY1, WindowCoordX2, WindowCoordY2, "");
  Info.Text(WindowCoordX1 + 5, WindowCoordY1 + 2, clrText, 
            FormatWidth(msg, WindowCoordX2 - WindowCoordX1 - 9).ptr());
  ProgX1  = WindowCoordX1 + 5; 
  ProgX2  = WindowCoordX2 - 5;
  ProgY   = WindowCoordY1 + 3;
  WinType = WIN_SCAN_PROGRESS;
  DrawScanProgress(ProgX1, ProgX2, ProgY, 0, 0);
  Info.Text(0, 0, 0, NULL);
  TitleBuf=GetTitle();
  ProgTitle=msg;
  SetTitle2(msg);
  Info.RestoreScreen(NULL);
}

// bugfixed by slst: bug #24
// New class member function
void FarProgress::SetScanProgressInfo(__int64 NumberOfFiles, __int64 TotalSize)
{
  if (WinType==WIN_SCAN_PROGRESS)
  {
    if (GetTime()-LastUpdate>TicksPerSec()/5)
    {
      DrawScanProgress(ProgX1, ProgX2, ProgY, NumberOfFiles, TotalSize);
      Info.Text(0, 0, 0, NULL);
      //SetTitle2(ProgTitle+" {"+String((int)(pc*100))+"%}");
      LastUpdate=GetTime();
    }
  }
}

// bugfixed by slst: bug #24
// New class member function
void FarProgress::DrawScanProgress(int x1, int x2, int y, __int64 NumberOfFiles, __int64 TotalSize)
{
  String FilesFmtStr = LOC("Status.FilesString") + " %-6I64d";
  wchar_t FilesStr[256];
  _snwprintf_s(FilesStr, 256, sizeof(FilesStr)/sizeof(wchar_t), (const wchar_t*)FilesFmtStr.ptr(), NumberOfFiles);

  String SizeFmtStr = LOC("Status.SizeString") + " %s";
  wchar_t SizeStr[256];
  _snwprintf_s(SizeStr, 256, sizeof(SizeStr)/sizeof(wchar_t), (const wchar_t*)SizeFmtStr.ptr(), (const wchar_t*)FormatValue(TotalSize).ptr());


  int s = x2 - x1 - (int)wcslen(SizeStr) - (int)wcslen(FilesStr);
  String spacer;
  if(s > 0)
	  spacer = String(' ', s);

  wchar_t buf[256];
  _snwprintf_s(buf, 256, sizeof(buf)/sizeof(wchar_t), L"%s %s%s", FilesStr, spacer.ptr(), SizeStr);

  Info.Text(x1, y + 1, clrText, buf);
  taskbar_icon.SetState(taskbar_icon.S_WORKING);
}
