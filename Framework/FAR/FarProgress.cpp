#include <stdhdr.h>
#include "../FileCopyEx/Common.h"
#include "lowlevelstr.h"
#include "strutils.h"
#include "far/farprogress.h"

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
  wchar_t buf[512];
  int attr=clrFrame;
  buf[W]=0;
  for (int i=0; i<W; i++) buf[i]=' ';
  Info.Text(X1, Y1, attr, buf);
  Info.Text(X1, Y2, attr, buf);
  buf[3]=0x2554;//'╔'
  buf[W-4]=0x2557;//'╗'
  for (int i=4; i<W-4; i++) buf[i]=0x2550;//'═'
  Info.Text(X1, Y1+1, attr, buf);
  buf[3]=0x255a;//'╚'
  buf[W-4]=0x255d;//'╝'
  Info.Text(X1, Y2-1, attr, buf);
  buf[3]=0x2551;//'║'
  buf[W-4]=0x2551;//'║'
  for (int i=4; i<W-4; i++) buf[i]=' ';
  for (int i=Y1+2; i<Y2-1; i++)
    Info.Text(X1, i, attr, buf);
  if (caption!="")
  {
    Info.Text(X1+(X2-X1-caption.len())/2, Y1+1, clrTitle, (String(" ")+caption+" ").ptr());
  }
  Info.Text(0, 0, 0, NULL);

  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  WORD a[512];
  ULONG cb;
  COORD cd = { X1+2, Y2+1 };
  ReadConsoleOutputAttribute(h, a, W, cd, &cb);
  for (int i=0; i<(int)cb; i++) a[i] = a[i] & 0x7;
  WriteConsoleOutputAttribute(h, a, cb, cd, &cb);
  for (int j=Y1+1; j<=Y2+1; j++)
  {
    COORD cd = { X2+1, j };
    ReadConsoleOutputAttribute(h, a, 2, cd, &cb);
    for (int i=0; i<(int)cb; i++) a[i] = a[i] & 0x7;
    WriteConsoleOutputAttribute(h, a, cb, cd, &cb);
  }
}

void FarProgress::GetConSize(int& w, int &h)
{
  HANDLE hc = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO bi;
  GetConsoleScreenBufferInfo(hc, &bi);
  w=bi.dwSize.X;
  h=bi.dwSize.Y;
}

void FarProgress::ShowMessage(const String& msg)
{
  Hide();
  int sw, sh;
  GetConSize(sw, sh);
  int W=msg.len()+12, H=5;
  int X1=(sw-W)/2; 
  int Y1=(sh-H)/2-2;
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
  int X1=(sw-W)/2; 
  int Y1=(sh-H)/2-2;
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
  SetTitle(v+" - Far");
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
  int WindowCoordX1 = (ConsoleWidth  - WindowWidth)/2;
  int WindowCoordY1 = (ConsoleHeight - WindowHeight)/2-2;
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

  size_t SizeStrOffset = x2 - x1 - wcslen(SizeStr) - wcslen(FilesStr);
  wchar_t Spacer[256];
  if (SizeStrOffset+1 >= sizeof(Spacer)) SizeStrOffset = 2;
  for(int i = 0; i < 256; ++i)
	  Spacer[i] = 0x20;
  Spacer[SizeStrOffset] = 0x00;

  wchar_t buf[256];
  _snwprintf_s(buf, 256, sizeof(buf)/sizeof(wchar_t), L"%s %s%s", FilesStr, Spacer, SizeStr);

  Info.Text(x1, y + 1, clrText, buf);
}
