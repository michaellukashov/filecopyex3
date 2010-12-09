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

#include <stdarg.h>
#include <tchar.h>

void _wtoacs(char *d, const wchar_t *s, size_t size);
void _atowcs(wchar_t *d, const char *s, size_t size);

void _tooem(char* s);
void _toansi(char* s);

wchar_t *_trim(wchar_t *s);
wchar_t *_trimquotes(wchar_t *s);

int _truestr(const wchar_t *s);

#define _atotcs(d, s, c) _atowcs(d, s, c)
#define _wtotcs(d, s, c) _wcopy(d, s, c)
#define _ttoacs(d, s, c) _wtoacs(d, s, c)
#define _ttowcs(d, s, c) _wcopy(d, s, c)

inline const wchar_t* _tcsend(const wchar_t* s) 
{
  wchar_t* res=(wchar_t*)s;
  while (*res) res++;
  return res;
}

inline const char* strend(const char* s)
{
  char* res=(char*)s;
  while (*res) res++;
  return res;
}

wchar_t* _tcsrstr(const wchar_t* wcs1, const wchar_t* wcs2);
wchar_t* _tcsrpbrk(const wchar_t* string, const wchar_t* control);
wchar_t* _tcsrspnp(const wchar_t* string, const wchar_t* control);

inline void _wcopy(wchar_t *d, const wchar_t *s, size_t size)
{
  wcsncpy_s(d, size, s, size);
  d[size-1]=0;
}

inline void _acopy(char *d, const char *s, size_t size)
{
  strncpy_s(d, size, s, size);
  d[size-1]=0;
}

inline void _tcopy(wchar_t *d, const wchar_t *s, size_t size)
{
  _tcsncpy_s(d, size, s, size);
  d[size-1]=0;
}

inline void _tcat(wchar_t *d, const wchar_t *s, size_t size)
{
  _tcsncat_s(d, size, s, size-_tcslen(d));
  d[size-1]=0;
}

