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

#include "objstring.h"

class FmtArg
{
public:
  String string;
  int size;
  union
  {
    int i;
    __int64 i64;
    double d;
    wchar_t c;
    void* p;
  } u;

  FmtArg(int v) { size=sizeof(int); u.i=v; }
  FmtArg(__int64 v) { size=sizeof(__int64); u.i64=v; }
  FmtArg(double v) { size=sizeof(double); u.d=v; }
  FmtArg(wchar_t v) { size=sizeof(wchar_t); u.c=v; }
  FmtArg(const String& v) { size=sizeof(void*); u.p=v.Lock(); string=v;}
  FmtArg(const wchar_t* v) { size=sizeof(void*); u.p=(void*)v; }
  FmtArg(const void* v) { size=sizeof(void*); u.p=(void*)v;}
};

String Format(const String& fmt,  
              const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, 
              const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, 
              const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, 
              const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, const FmtArg& =0);

String FormatNum(__int64);
String FormatTime(const FILETIME&);

String Replace(const String&, const String&, const String&);

String FormatProgress(__int64 cb, __int64 total);
String FormatSpeed(__int64 cb);
String FormatValue(__int64 Value);
