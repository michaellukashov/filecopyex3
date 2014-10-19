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

#include "StdHdr.h"
#include "ObjString.h"

static bool isbadchar(wchar_t c)
{
  return c == L'\0' || c == L'\t' ||
    c == L'\n' || c == L'\r' ||
    c == L' ';
}

String::String(const char * v)
{
  if (v != nullptr && v[0] != '\0')
  {
    size_t len = strlen(v);
    str.resize(len);
    if (::MultiByteToWideChar(CP_OEMCP, 0, v, (int)len,
      &(*str.begin()), (int)len) == 0)
    {
      str.clear();
    }
  }
}

String String::substr(size_t s, size_t l) const
{
  if (s >= len())
  {
    return String();
  }
  return String(str.substr(s, l));
}

String String::trim() const
{
  intptr_t start = 0;
  intptr_t end = len() - 1;
  while (start <= end && isbadchar((*this)[start]))
  {
    start++;
  }
  while (end >= start && isbadchar((*this)[end]))
  {
    end--;
  }
  return substr(start, end - start + 1);
}

String String::ltrim() const
{
  intptr_t start = 0;
  intptr_t end = len() - 1;
  while (start <= end && isbadchar((*this)[start]))
  {
    start++;
  }
  return substr(start, end - start + 1);
}

String String::rtrim() const
{
  intptr_t start = 0;
  intptr_t end = len() - 1;
  while (end >= start && isbadchar((*this)[end]))
  {
    end--;
  }
  return substr(start, end - start + 1);
}

String String::trimquotes() const
{
  intptr_t start = 0;
  intptr_t end = len() - 1;
  while (start <= end && ((*this)[start]) == L'"')
  {
    start++;
  }
  while (end >= start && ((*this)[end]) == L'"')
  {
    end--;
  }
  return substr(start, end - start + 1);
}

String String::rev() const
{
  String res;
  res.str.assign(str.rend(), str.rbegin());
  return res;
}

String String::replace(const String & what, const String & with) const
{
  if (what.empty())
  {
    return *this;
  }
  String res;
  intptr_t start = 0;
  intptr_t p;
  while ((p = find(what, start)) != -1)
  {
    res += substr(start, p - start);
    res += with;
    start = p + what.len();
  }
  res += substr(start);
  return res;
}

String String::toUpper() const
{
  String res(str);
  ::CharUpperBuff((LPTSTR)res.c_str(), (DWORD)len());
  return res;
}

String String::toLower() const
{
  String res(str);
  ::CharLowerBuff((LPTSTR)res.c_str(), (DWORD)len());
  return res;
}

size_t npos_minus1(size_t pos)
{
  return (pos == std::string::npos) ? (size_t)-1 : pos;
}
