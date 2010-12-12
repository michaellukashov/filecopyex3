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
#include "lowlevelstr.h"

void _atowcs(wchar_t *d, size_t size, const char *s)
{
  MultiByteToWideChar(CP_ACP, 0, s, -1, d, (int)size);
  d[size-1]=0;
}

wchar_t *_trim(wchar_t *arg)
{
  while (*arg && (*arg==' '||*arg=='\t'||*arg=='\n'||*arg=='\r')) arg++;
  wchar_t *p=arg+wcslen(arg)-1;
  while (p>=arg && (*p==' '||*p=='\t'||*p=='\n'||*p=='\r')) *p--=0;
  return arg;
}

wchar_t *_trimquotes(wchar_t *arg)
{
  if (*arg && (*arg=='"')) arg++;
  wchar_t *p=arg+wcslen(arg)-1;
  if (p>=arg && (*p=='"')) *p--=0;
  return arg;
}

int _truestr(const wchar_t *s)
{
  return s && (!wcscmp(s, L"1") || !_wcsicmp(s, L"yes") || !_wcsicmp(s, L"true"));
}

wchar_t* _tcsrstr(const wchar_t* wcs1, const wchar_t* wcs2)
{
  wchar_t *cp=(wchar_t*)_tcsend(wcs1)-1;
  wchar_t *s1, *s2;
  if (!*wcs2) return cp;
  while (cp>=wcs1)
  {
    s1=cp;
    s2=(wchar_t*) wcs2;
    while (*s1 && *s2 && !(*s1-*s2)) s1++, s2++;
    if (!*s2) return cp;
    cp--;
  }
  return NULL;
}

wchar_t* _tcsrpbrk(const wchar_t* string, const wchar_t* control)
{
  wchar_t *wcset;
  wchar_t *str=(wchar_t*)_tcsend(string)-1;
  /* 1st char in control string stops search */
  while (str>=string) 
  {
    for (wcset=(wchar_t*)control; *wcset; wcset++) 
      if (*wcset==*str) 
        return str;
    str--;
  }
  return NULL;
}

wchar_t* _tcsrspnp(const wchar_t* string, const wchar_t* control)
{
  wchar_t *str=(wchar_t*)_tcsend(string)-1;
  wchar_t *ctl;
  /* 1st char not in control string stops search */
  while (str>=string) 
  {
    for (ctl=(wchar_t*)control; *ctl!=*str; ctl++) 
      if (*ctl==(wchar_t)0) 
        /* reached end of control string without finding a match */
        return str;
    str--;
  }
  /* The whole string consisted of characters from control */
  return NULL;
}
