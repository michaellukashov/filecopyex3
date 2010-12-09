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

#include "stdhdr.h"
#include <stdarg.h>
//#include "../FileCopyEx/Common.h"
#include "FAR/FarPlugin.h"
#include "strutils.h"


String Format(const String& fmt, 
              const FmtArg& a1, const FmtArg& a2, const FmtArg& a3, const FmtArg& a4, 
              const FmtArg& a5, const FmtArg& a6, const FmtArg& a7, const FmtArg& a8, 
              const FmtArg& a9, const FmtArg& a10, const FmtArg& a11, const FmtArg& a12, 
              const FmtArg& a13, const FmtArg& a14, const FmtArg& a15, const FmtArg& a16)
{
  wchar_t buf[8192], *p=fmt.Lock();
  char *arglist=(char*)_alloca(__max(__max(sizeof(int), 
    sizeof(double)), sizeof(void*))*16),
    *argptr=arglist;
#define ROUND(n) ((n+sizeof(int)-1)&~(sizeof(int)-1))
#define ADD(a) { \
    memcpy(argptr, &a.u, a.size); \
    argptr+=ROUND(a.size); }
  ADD(a1); ADD(a2); ADD(a3); ADD(a4); ADD(a5); ADD(a6); ADD(a7); ADD(a8);
  ADD(a9); ADD(a10); ADD(a11); ADD(a12); ADD(a13); ADD(a14); ADD(a15); ADD(a16);
#undef ADD
#undef ROUND
  _vsntprintf_s(buf, 8192, p, (va_list)arglist);
  fmt.Unlock();
  return buf;
}

String FormatNum(__int64 n)
{
  wchar_t num[64], buf[64];
  _stprintf_s(num, 64, _T("%I64d"), n);
  wchar_t *s=_tcsrev(num), *d=buf;
  int q=0;
  while (*s) 
  {
    *d++=*s++; 
    // Bug #9 fixed by axxie
    if (!(++q % 3)) *d++=' ';
  }
  // Bug #9 fixed by axxie
  if (*(d-1)==' ') d--;
  *d = 0;
  _tcsrev(buf);
  return buf;
}

String FormatTime(const FILETIME& ft)
{
  FILETIME ft1;
  SYSTEMTIME st;
  FileTimeToLocalFileTime(&ft, &ft1);
  FileTimeToSystemTime(&ft1, &st);
  return Format("%02d.%02d.%04d %02d:%02d:%02d",
                (int)st.wDay, (int)st.wMonth, (int)st.wYear, 
                (int)st.wHour, (int)st.wMinute, (int)st.wSecond);
}

String Replace(const String& s, const String& s1, const String& s2)
{
  String src=s, res;
  int p;
  while ((p=src.find(s1))!=-1)
  {
    res+=src.substr(0, p)+s2;
    src=src.substr(p+s1.len());
  }
  res+=src;
  return res;
}


String FormatProgress(__int64 cb, __int64 total)
{
  __int64 n=total;
  int pw=0;
  __int64 div=1;
  while (n>=100000) { div*=1024; n/=1024; pw++; }
  String un;
  switch (pw) 
  {
    case 0: un=LOC("Engine.Bytes"); break;
    case 1: un=LOC("Engine.Kb"); break;
    case 2: un=LOC("Engine.Mb"); break;
    case 3: un=LOC("Engine.Gb"); break;
    case 4: un=LOC("Engine.Tb"); break;
    case 5: un=LOC("Engine.Pb"); break;
  }
  return Format("%s %s %s %s [%d%%]", FormatNum(cb/div), LOC("Engine.Of"), 
                FormatNum(total/div), un, (int)(total?(float)cb/total*100:0));
}


String FormatSpeed(__int64 cb)
{
  __int64 n=cb;
  int pw=0;
  __int64 div=1;
  while (n>=100000) { div*=1024; n/=1024; pw++; }
  String un;
  switch (pw) 
  {
    case 0: un=LOC("Engine.Bytes"); break;
    case 1: un=LOC("Engine.Kb"); break;
    case 2: un=LOC("Engine.Mb"); break;
    case 3: un=LOC("Engine.Gb"); break;
    case 4: un=LOC("Engine.Tb"); break;
    case 5: un=LOC("Engine.Pb"); break;
  }

  return Format("%s %s/%s", FormatNum(cb/div), un, LOC("Engine.Sec"));
}


String FormatValue(__int64 Value)
{
  int pw = 0;
  __int64 div = 1;
  while (Value >= 100000) { div *= 1024; Value /= 1024; pw++; }
  String UnitStr;
  switch (pw) 
  {
    case 0: UnitStr = LOC("Engine.Bytes"); break;
    case 1: UnitStr = LOC("Engine.Kb"); break;
    case 2: UnitStr = LOC("Engine.Mb"); break;
    case 3: UnitStr = LOC("Engine.Gb"); break;
    case 4: UnitStr = LOC("Engine.Tb"); break;
    case 5: UnitStr = LOC("Engine.Pb"); break;
  }
  return Format("%s %s", FormatNum(Value), UnitStr);
}

