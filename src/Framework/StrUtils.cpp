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

#include <stdarg.h>
#include "ObjString.h"
#include "../Common.h"

String Format(const wchar_t * fmt, ...)
{
  wchar_t buf[8192];
  va_list args;
  va_start(args, fmt);
  _vsnwprintf_s(buf, LENOF(buf), fmt, args);
  va_end(args);
  return buf;
}

String FormatNum(int64_t n)
{
  static bool first = true;
  static NUMBERFMT fmt;
  static wchar_t DecimalSep[4];
  static wchar_t ThousandSep[4];

  if (first)
  {
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, ThousandSep, ARRAYSIZE(ThousandSep));
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, DecimalSep, ARRAYSIZE(DecimalSep));
    DecimalSep[1] = 0; //В винде сепараторы цифр могут быть больше одного символа
    ThousandSep[1] = 0; //но для нас это будет не очень хорошо
    /*
    if (LOWORD(Global->Opt->FormatNumberSeparators)) {
      *DecimalSep=LOWORD(Global->Opt->FormatNumberSeparators);
    }

    if (HIWORD(Global->Opt->FormatNumberSeparators)) {
      *ThousandSep=HIWORD(Global->Opt->FormatNumberSeparators);
    }
    */

    fmt.LeadingZero = 1;
    fmt.Grouping = 3;
    fmt.lpDecimalSep = DecimalSep;
    fmt.lpThousandSep = ThousandSep;
    fmt.NegativeOrder = 1;
    fmt.NumDigits = 0;
    first = false;
  }

  String strSrc(n);
  int size = GetNumberFormat(LOCALE_USER_DEFAULT, 0, strSrc.c_str(), &fmt, nullptr, 0);
  wchar_t * lpwszDest = new wchar_t[size];
  GetNumberFormat(LOCALE_USER_DEFAULT, 0, strSrc.c_str(), &fmt, lpwszDest, size);
  String strDest(lpwszDest);
  delete[] lpwszDest;

  return strDest;
}

String FormatTime(const FILETIME & ft)
{
  FILETIME ft1;
  SYSTEMTIME st;
  FileTimeToLocalFileTime(&ft, &ft1);
  FileTimeToSystemTime(&ft1, &st);
  return Format(L"%02d.%02d.%04d %02d:%02d:%02d",
                (int)st.wDay, (int)st.wMonth, (int)st.wYear,
                (int)st.wHour, (int)st.wMinute, (int)st.wSecond);
}

String FormatProgress(int64_t cb, int64_t total)
{
  int64_t n = total;
  int pw = 0;
  int64_t div = 1;
  while (n > 999999)
  {
    div *= 1024;
    n /= 1024;
    pw++;
  }
  String un;
  switch (pw)
  {
    case 0: un=LOC(L"Engine.Bytes"); break;
    case 1: un=LOC(L"Engine.Kb"); break;
    case 2: un=LOC(L"Engine.Mb"); break;
    case 3: un=LOC(L"Engine.Gb"); break;
    case 4: un=LOC(L"Engine.Tb"); break;
    case 5: un=LOC(L"Engine.Pb"); break;
  }
  return Format(L"%s %s %s %s [%d%%]", FormatNum(cb / div).ptr(), LOC(L"Engine.Of").ptr(),
                FormatNum(total / div).ptr(), un.ptr(), (int)(total ? (float)cb / total * 100 : 0));
}

String FormatSpeed(int64_t cb)
{
  int64_t n = cb;
  int pw = 0;
  int64_t div = 1;
  while (n >= 100000)
  {
    div *= 1024;
    n /= 1024;
    pw++;
  }
  String un;
  switch (pw)
  {
    case 0: un=LOC(L"Engine.Bytes"); break;
    case 1: un=LOC(L"Engine.Kb"); break;
    case 2: un=LOC(L"Engine.Mb"); break;
    case 3: un=LOC(L"Engine.Gb"); break;
    case 4: un=LOC(L"Engine.Tb"); break;
    case 5: un=LOC(L"Engine.Pb"); break;
  }

  return Format(L"%s %s/%s", FormatNum(cb / div).ptr(), un.ptr(), LOC(L"Engine.Sec").ptr());
}

String FormatValue(int64_t Value)
{
  int pw = 0;
  int64_t div = 1;
  while (Value >= 100000) { div *= 1024; Value /= 1024; pw++; }
  String UnitStr;
  switch (pw)
  {
    case 0: UnitStr = LOC(L"Engine.Bytes"); break;
    case 1: UnitStr = LOC(L"Engine.Kb"); break;
    case 2: UnitStr = LOC(L"Engine.Mb"); break;
    case 3: UnitStr = LOC(L"Engine.Gb"); break;
    case 4: UnitStr = LOC(L"Engine.Tb"); break;
    case 5: UnitStr = LOC(L"Engine.Pb"); break;
  }
  return Format(L"%s %s", FormatNum(Value).ptr(), UnitStr.ptr());
}
