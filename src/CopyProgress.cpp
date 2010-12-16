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

#include "../framework/stdhdr.h"
#include "common.h"
#include "copyprogress.h"
#include "../framework/strutils.h"

const int W = 64, H = 13, MG = 5;

CopyProgress::CopyProgress(void)
{
  // bug #22 fixed by axxie
  lastupdate=lastupdate_read=lastupdate_write=0;
  interval=TicksPerSec()/10;
  clastupdate=0;
  cinterval=TicksPerSec();
  int w, h;
  GetConSize(w, h);
  X1=(w-W + 1)/2;
  Y1=(h-H - 1)/2;
  X2=X1+W-1, Y2=Y1+H-1;
}

CopyProgress::~CopyProgress(void)
{
}

void CopyProgress::Start(int move)
{
  hScreen=Info.SaveScreen(0, 0, -1, -1);
  DrawWindow(X1, Y1, X2, Y2, move? LOC("Engine.Moving"):LOC("Engine.Copying"));
  wchar_t buf[512], *p=buf;
  for (int i=0; i<W-MG*2+2; i++) *p++=0x2500;//'─'
  *p=0;
  Info.Text(X1+MG-1, Y1+5, clrFrame, buf);
  Info.Text(X1+MG-1, Y1+9, clrFrame, buf);
  Move=move;
  Info.Text(0, 0, 0, NULL);
  Info.RestoreScreen(NULL);
  TitleBuf=GetTitle();
}

void CopyProgress::Stop()
{
  Info.RestoreScreen(NULL);
  Info.RestoreScreen(hScreen);
}


void CopyProgress::DrawTime(__int64 ReadBytes, __int64 WriteBytes, __int64 TotalBytes,
                            __int64 ReadTime, __int64 WriteTime,
                            __int64 ReadN, __int64 WriteN, __int64 TotalN,
                            int ParallelMode, __int64 FirstWriteTime, 
                            __int64 StartTime, int BufferSize)
{
  double TotalTime     = 0;
  double ElapsedTime   = (double)(GetTime()-StartTime);
  double RemainingTime = 0;

  //DebugLog(_T("RBytes: %I64d  WBytes: %I64d  RdTime: %I64d  WrTime: %I64d  StTime: %I64d\n"),
  //          ReadBytes, WriteBytes, ReadTime, WriteTime, StartTime);

  const __int64 MinRWValue = 0x10000;
  
  // bugfixed by slst: bug #18
  if (((ReadBytes > MinRWValue)  && (WriteBytes > MinRWValue)) || 
      ((TotalBytes < BufferSize) && (ReadBytes > MinRWValue))) // if selected files size < buffer size
  {
    double ReadSpeed  = (ReadTime>0)  ? (double)ReadBytes  / (double)ReadTime  : 0; // bytes per tick
    double WriteSpeed = (WriteTime>0) ? (double)WriteBytes / (double)WriteTime : 0; // bytes per tick

    double ReadTimeRemain   = (ReadSpeed  > 0.001) ? ((double)TotalBytes - (double)ReadBytes)  / ReadSpeed  : 0;
    double WriteTimeRemain  = (WriteSpeed > 0.001) ? ((double)TotalBytes - (double)WriteBytes) / WriteSpeed : 0;

    //DebugLog(_T("RSpeed: %4.4f  WSpeed: %4.4f  RTRemain: %4.4f  WTRemain: %4.4f\n"),
    //        ReadSpeed, WriteSpeed, ReadTimeRemain, WriteTimeRemain);

    if (ParallelMode)
    { // parallel mode
      double BufferWriteTime = 0;
      if (ReadBytes == TotalBytes)
        BufferWriteTime = WriteTimeRemain;
      else
        BufferWriteTime = (WriteSpeed > 0.001) ? ((double)BufferSize / WriteSpeed) : 0;   
      //DebugLog(_T("BufferWriteTime: %3.2f\n"), BufferWriteTime / TicksPerSec());

      if (WriteSpeed > 0) // writing is started
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
    { // non-parallel mode
      RemainingTime = ReadTimeRemain + WriteTimeRemain;
    }

    TotalTime = ElapsedTime + RemainingTime;
  }
  
  if (TotalTime     < 0) TotalTime     = 0;
  if (ElapsedTime   < 0) ElapsedTime   = 0;
  if (RemainingTime < 0) RemainingTime = 0;
    
  TotalTime     /= TicksPerSec();
  ElapsedTime   /= TicksPerSec();
  RemainingTime /= TicksPerSec();

  // debug
  //DebugLog(_T("Total: %4.1f  Elapsed: %4.1f  Remain: %4.1f\n"), TotalTime, ElapsedTime, RemainingTime);
  //DebugLog(_T("---------------------------------------------------------------------------\n"));

  int l = X1+MG;

  String buf;
  buf = LOC("Engine.Total");
  DrawText(l, Y1+10, clrLabel, buf);
  l += buf.len();
  buf = Format(L" %2.2d:%2.2d  ", (int)TotalTime/60, (int)TotalTime%60);
  DrawText(l, Y1+10, clrText, buf);
  l += buf.len();

  buf = LOC("Engine.Elapsed");
  DrawText(l, Y1+10, clrLabel, buf);
  l += buf.len();
  buf = Format(L" %2.2d:%2.2d  ", (int)ElapsedTime/60, (int)ElapsedTime%60);
  DrawText(l, Y1+10, clrText, buf);
  l += buf.len();

  String buf1;
  buf = String("        ")+LOC("Engine.Remaining");
  buf1 = Format(L" %2.2d:%2.2d", (int)RemainingTime/60, (int)RemainingTime%60);
  DrawText(X2-MG-buf1.len()-buf.len()+1, Y1+10, clrLabel, buf);
  DrawText(X2-MG-buf1.len()+1, Y1+10, clrText, buf1);

  if (GetTime()-clastupdate > cinterval)
  {
    clastupdate = GetTime();
    int pc=TotalBytes?(int)((float)(ReadBytes+WriteBytes)/(TotalBytes*2)*100):0;
    if (pc<0) pc=0;
    if (pc>100) pc=100;
    buf = Format(L"{%d%% %2.2d:%2.2d} %s", pc, (int)RemainingTime/60, (int)RemainingTime%60, 
                Move? LOC("Engine.Moving").ptr():LOC("Engine.Copying").ptr());
	SetTitle2(buf);
  }
}


void CopyProgress::DrawProgress(const String& pfx, int y, __int64 cb, __int64 total,
                                __int64 time, __int64 n, __int64 totaln)
{
  if (cb>total) cb=total;
  DrawText(X1+MG, Y1+y, clrLabel, pfx);
  String buf;
  buf= FormatWidth(FormatProgress(cb, total), W-MG*2-pfx.len());
  DrawText(X1+MG+pfx.len()+1, Y1+y, clrText, buf);

  FarProgress::DrawProgress(X1+MG, X2-MG, Y1+y+1, total?((float)cb/total):0);

  __int64 rate = (__int64) (time ? (float)cb / time * TicksPerSec() : 0);
  buf= FormatSpeed(rate);
  DrawText(X2-MG-buf.len()+1, Y1+y, clrText, buf);
}

void CopyProgress::DrawName(const String& fn, int y)
{
  DrawText(X1+MG, Y1+y, clrText, FormatWidth(fn, W-MG*2));
}

void CopyProgress::ShowReadName(const String& fn)
{
  // bug #22 fixed by axxie
  if (GetTime()-lastupdate_read > interval)
  {
    lastupdate_read = GetTime();
    DrawName(fn, 4);
    Info.Text(0, 0, 0, NULL);
  }
}

void CopyProgress::ShowWriteName(const String& fn)
{
  // bug #22 fixed by axxie
  if (GetTime()-lastupdate_write > interval)
  {
    lastupdate_write = GetTime();
    DrawName(fn, 8);
    Info.Text(0, 0, 0, NULL);
  }
}

void CopyProgress::ShowProgress(__int64 read, __int64 write, __int64 total,
                                __int64 readTime, __int64 writeTime,
                                __int64 readN, __int64 writeN,
                                __int64 totalN, int parallel, 
                                __int64 FirstWrite, __int64 StartTime, int BufferSize)
{
  if (GetTime()-lastupdate > interval)
  {
    lastupdate = GetTime();
    DrawProgress(LOC("Engine.Reading"), 2, read, total, readTime, readN, totalN);
    DrawProgress(LOC("Engine.Writing"), 6, write, total, writeTime, writeN, totalN);
    DrawTime(read, write, total, readTime, writeTime, readN, writeN, totalN,
              parallel, FirstWrite, StartTime, BufferSize);
    Info.Text(0, 0, 0, NULL);
  }
}
