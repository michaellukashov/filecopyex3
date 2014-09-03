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
#include "Common.h"
#include "CopyProgress.h"
#include "tools.h"
#include "ui.h"

const int W = 64, H = 13, MG = 5;

CopyProgress::CopyProgress(void)
{
  lastupdate = lastupdate_read = lastupdate_write = 0;
  interval = TicksPerSec() / 10;
  clastupdate = 0;
  cinterval = TicksPerSec();
  int w, h;
  GetConSize(w, h);
  X1 = (w - W + 1) / 2;
  Y1 = (h - H - 1) / 2;
  X2 = X1 + W - 1, Y2 = Y1 + H - 1;
  Move = false;
}

CopyProgress::~CopyProgress(void)
{
}

void CopyProgress::Start(bool move)
{
  Move = move;
  hScreen = Info.SaveScreen(0, 0, -1, -1);
  RedrawWindowIfNeeded();
}

void CopyProgress::Stop()
{
  Info.RestoreScreen(nullptr);
  Info.RestoreScreen(hScreen);
}


void CopyProgress::DrawTime(int64_t ReadBytes, int64_t WriteBytes, int64_t TotalBytes,
                            int64_t ReadTime, int64_t WriteTime,
                            int64_t ReadN, int64_t WriteN, int64_t TotalN,
                            int ParallelMode, int64_t FirstWriteTime,
                            int64_t StartTime, size_t BufferSize)
{
  RedrawWindowIfNeeded();
  double TotalTime = 0;
  double ElapsedTime = (double)(GetTime() - StartTime);
  double RemainingTime = 0;

  //DebugLog(_T("RBytes: %I64d  WBytes: %I64d  RdTime: %I64d  WrTime: %I64d  StTime: %I64d\n"),
  //          ReadBytes, WriteBytes, ReadTime, WriteTime, StartTime);

  const int64_t MinRWValue = 0x10000;

  if (((ReadBytes > MinRWValue) && (WriteBytes > MinRWValue)) ||
      ((TotalBytes < (int64_t)BufferSize) && (ReadBytes > MinRWValue)))
  {
    double ReadSpeed  = (ReadTime > 0)  ? (double)ReadBytes  / (double)ReadTime  : 0; // bytes per tick
    double WriteSpeed = (WriteTime > 0) ? (double)WriteBytes / (double)WriteTime : 0; // bytes per tick

    double ReadTimeRemain  = (ReadSpeed  > 0.001) ? ((double)TotalBytes - (double)ReadBytes)  / ReadSpeed  : 0;
    double WriteTimeRemain = (WriteSpeed > 0.001) ? ((double)TotalBytes - (double)WriteBytes) / WriteSpeed : 0;

    //DebugLog(_T("RSpeed: %4.4f  WSpeed: %4.4f  RTRemain: %4.4f  WTRemain: %4.4f\n"),
    //        ReadSpeed, WriteSpeed, ReadTimeRemain, WriteTimeRemain);

    if (ParallelMode)
    {
      double BufferWriteTime = 0;
      if (ReadBytes == TotalBytes)
        BufferWriteTime = WriteTimeRemain;
      else
        BufferWriteTime = (WriteSpeed > 0.001) ? ((double)BufferSize / WriteSpeed) : 0;
      //DebugLog(_T("BufferWriteTime: %3.2f\n"), BufferWriteTime / TicksPerSec());

      if (WriteSpeed > 0)
      {
        if (ReadSpeed < WriteSpeed)
          RemainingTime = ReadTimeRemain + BufferWriteTime;
        else
          RemainingTime = WriteTimeRemain + (double)FirstWriteTime;
      }
      else
      {
        RemainingTime = ReadTimeRemain + BufferWriteTime;
      }
    }
    else
    {
      // non-parallel mode
      RemainingTime = ReadTimeRemain + WriteTimeRemain;
    }

    TotalTime = ElapsedTime + RemainingTime;
  }

  if (TotalTime < 0)
    TotalTime = 0;
  if (ElapsedTime < 0)
    ElapsedTime = 0;
  if (RemainingTime < 0)
    RemainingTime = 0;

  TotalTime     /= TicksPerSec();
  ElapsedTime   /= TicksPerSec();
  RemainingTime /= TicksPerSec();

  //DebugLog(_T("Total: %4.1f  Elapsed: %4.1f  Remain: %4.1f\n"), TotalTime, ElapsedTime, RemainingTime);
  //DebugLog(_T("---------------------------------------------------------------------------\n"));

  intptr_t l = X1 + MG;

  String buf;
  buf = LOC(L"Engine.Total");
  Text(l, Y1 + 10, &clrLabel, buf);
  l += buf.len();
  buf = Format(L" %2.2d:%2.2d  ", (int)TotalTime / 60, (int)TotalTime % 60);
  Text(l, Y1 + 10, &clrText, buf);
  l += buf.len();

  buf = LOC(L"Engine.Elapsed");
  Text(l, Y1 + 10, &clrLabel, buf);
  l += buf.len();
  buf = Format(L" %2.2d:%2.2d  ", (int)ElapsedTime / 60, (int)ElapsedTime % 60);
  Text(l, Y1 + 10, &clrText, buf);

  String buf1;
  buf = String(L"        ") + LOC(L"Engine.Remaining");
  buf1 = Format(L" %2.2d:%2.2d", (int)RemainingTime / 60, (int)RemainingTime % 60);
  Text(X2 - MG - buf1.len() - buf.len() + 1, Y1 + 10, &clrLabel, buf);
  Text(X2 - MG - buf1.len() + 1, Y1 + 10, &clrText, buf1);

  int64_t tm = GetTime();
  if (tm - clastupdate > cinterval)
  {
    clastupdate = tm;
    int pc = TotalBytes ? (int)((float)(ReadBytes + WriteBytes) / (TotalBytes * 2) * 100) : 0;
    if (pc < 0)
      pc = 0;
    if (pc > 100)
      pc = 100;
    buf = Format(L"{%d%% %2.2d:%2.2d} %s", pc, (int)RemainingTime / 60, (int)RemainingTime % 60,
                 Move ? LOC(L"Engine.Moving").ptr() : LOC(L"Engine.Copying").ptr());
    SetTitle2(buf);
  }
}


void CopyProgress::DrawProgress(const String & pfx, int y, int64_t cb, int64_t total,
                                int64_t time, int64_t n, int64_t totaln)
{
  if (cb > total)
    cb = total;
  Text(X1 + MG, Y1 + y, &clrLabel, pfx);
  String buf;
  buf = FormatWidth(FormatProgress(cb, total), W - MG * 2 - pfx.len());
  Text(X1 + MG + pfx.len() + 1, Y1 + y, &clrText, buf);

  FarProgress::DrawProgress(X1 + MG, X2 - MG, Y1 + y + 1, total ? ((float)cb / total) : 0);

  int64_t rate = (int64_t)(time ? (float)cb / time * TicksPerSec() : 0);
  buf = FormatSpeed(rate);
  Text(X2 - MG - buf.len() + 1, Y1 + y, &clrText, buf);
}

void CopyProgress::DrawName(const String & fn, int y)
{
  Text(X1 + MG, Y1 + y, &clrText, FormatWidth(fn, W - MG * 2));
}

void CopyProgress::ShowReadName(const String & fn)
{
  int64_t tm = GetTime();
  if (tm - lastupdate_read > interval)
  {
    RedrawWindowIfNeeded();
    lastupdate_read = tm;
    DrawName(fn, 4);
    Info.Text(0, 0, 0, nullptr);
  }
}

void CopyProgress::ShowWriteName(const String & fn)
{
  int64_t tm = GetTime();
  if (tm - lastupdate_write > interval)
  {
    RedrawWindowIfNeeded();
    lastupdate_write = tm;
    DrawName(fn, 8);
    Info.Text(0, 0, 0, nullptr);
  }
}

void CopyProgress::ShowProgress(int64_t read, int64_t write, int64_t total,
                                int64_t readTime, int64_t writeTime,
                                int64_t readN, int64_t writeN,
                                int64_t totalN, int parallel,
                                int64_t FirstWrite, int64_t StartTime, size_t BufferSize)
{
  int64_t tm = GetTime();
  if (tm - lastupdate > interval)
  {
    RedrawWindowIfNeeded();
    lastupdate = tm;
    DrawProgress(LOC(L"Engine.Reading"), 2, read, total, readTime, readN, totalN);
    DrawProgress(LOC(L"Engine.Writing"), 6, write, total, writeTime, writeN, totalN);
    DrawTime(read, write, total, readTime, writeTime, readN, writeN, totalN,
             parallel, FirstWrite, StartTime, BufferSize);
    Info.Text(0, 0, 0, nullptr);
  }
}

void CopyProgress::RedrawWindowIfNeeded()
{
  if (NeedToRedraw)
  {
    RedrawWindow();
    NeedToRedraw = false;
  }
}

void CopyProgress::RedrawWindow()
{
  DrawWindow(X1, Y1, X2, Y2, Move ? LOC(L"Engine.Moving") : LOC(L"Engine.Copying"));
  wchar_t buf[512];
  wchar_t * p = buf;
  for (int Index = 0; Index < W - MG * 2 + 2; Index++)
    *p++ = 0x2500; //'─'
  *p = 0;
  Info.Text(X1 + MG - 1, Y1 + 5, &clrFrame, buf);
  Info.Text(X1 + MG - 1, Y1 + 9, &clrFrame, buf);
  Info.Text(0, 0, 0, nullptr);
  Info.RestoreScreen(nullptr);
  TitleBuf = GetTitle();
}
