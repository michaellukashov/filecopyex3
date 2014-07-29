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

#include <string>
#include <stdint.h>

#include "LowLevelStr.h"

size_t npos_minus1(size_t pos);

class String
{
public:
  String() {};

  String(const char * v)
  {
    std::string s(v);
    str.assign(s.begin(), s.end());
  }

  String(wchar_t ch, intptr_t len)
  {
    str.resize(len, ch);
  }

  String(const wchar_t * v) : str(v)
  {
  }

  String(const std::wstring & v) : str(v)
  {
  }

  inline wchar_t operator[](intptr_t i) const
  {
    if (i >= 0 && i < (int)str.length()) return str[i];
    else return 0;
  }

  explicit String(int v)
  {
    wchar_t buf[64];
    _itow_s(v, buf, 64, 10);
    str = buf;
  }

  explicit String(size_t v)
  {
    wchar_t buf[64];
    _i64tow_s((int64_t)v, buf, 64, 10);
    str = buf;
  }

  explicit String(int64_t v)
  {
    wchar_t buf[64];
    _i64tow_s(v, buf, 64, 10);
    str = buf;
  }

  explicit String(float v)
  {
    wchar_t buf[64];
    swprintf_s(buf, 64, L"%g", v);
    str = buf;
  }

  explicit String(bool v)
  {
    str = v ? L"1" : L"0";
  }

  inline bool operator==(const String & v) const { return cmp(v) == 0; }
  inline bool operator!=(const String & v) const { return cmp(v) != 0; }
  inline bool operator<(const String & v) const { return cmp(v) < 0; }
  inline bool operator>(const String & v) const { return cmp(v) > 0; }
  inline bool operator<=(const String & v) const { return cmp(v) <= 0; }
  inline bool operator>=(const String & v) const { return cmp(v) >= 0; }
  inline void operator+=(const String & v) { str += v.str; }
  inline void operator+=(wchar_t c) { str += c; }
  const String operator+(const String & v) const { return str + v.str; }

  inline size_t len() const { return str.length(); }
  inline bool empty() const { return str.empty(); }

  int AsInt() const { return _wtoi(ptr()); }
  int64_t AsInt64() const { return _wtoi64(ptr()); }
  float AsFloat() const { return (float)_wtof(ptr()); }
  bool AsBool() const { return (*this) == L"1"; }
  void copyTo(wchar_t * buf, size_t sz) const { wcscpy_s(buf, sz, ptr()); }

  int cmp(const String & v) const { return ncmp(v, 0x7FFFFFFF); }
  int icmp(const String & v) const { return nicmp(v, 0x7FFFFFFF); }
  int ncmp(const String & v, size_t sz) const { return wcsncmp(ptr(), v.ptr(), sz); }
  int nicmp(const String & v, size_t sz) const { return _wcsnicmp(ptr(), v.ptr(), sz); }
  // bug #46 fixed by axxie

  String substr(size_t pos = 0, size_t len = std::string::npos) const;

  const String left(size_t n) const { return substr(0, n); }
  const String right(size_t n) const { return substr(len() - n, n); }

  String trim() const;
  String ltrim() const;
  String rtrim() const;
  String trimquotes() const;

  String rev() const;

  String replace(const String & what, const String & with) const;
  String toUpper() const;
  String toLower() const;

  size_t find(const String & v, size_t start = 0) const { return npos_minus1(str.find(v.str, start)); }
  size_t find(wchar_t v, size_t start = 0) const { return npos_minus1(str.find(v, start)); }
  size_t find_first_of(const String & v, size_t start = 0) const { return npos_minus1(str.find_first_of(v.str, start)); }
  size_t rfind(const String & v, size_t start = std::string::npos) const { return npos_minus1(str.rfind(v.str, start)); }
  size_t rfind(wchar_t v, size_t start = std::string::npos) const { return npos_minus1(str.rfind(v, start)); }
  size_t find_last_of(const String & v, size_t start = std::string::npos) const { return npos_minus1(str.find_last_of(v.str, start)); }

  const wchar_t * ptr() const { return c_str(); }
  const wchar_t * c_str() const { return str.c_str(); }

  void reserve(size_t n = 0) { str.reserve(n); }

private:
  std::wstring str;
};

extern const String emptyString;
