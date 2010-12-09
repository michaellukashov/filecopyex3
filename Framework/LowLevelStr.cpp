#include <stdhdr.h>
#include "lowlevelstr.h"

void _wtoacs(char *d, const wchar_t *s, size_t size)
{
#ifndef UNICODE
  wcstombs(d, s, size);
#else
  WideCharToMultiByte(CP_ACP, 0, s, -1, d, (int)size, NULL, NULL);
#endif
  d[size-1]=0;
}

void _atowcs(wchar_t *d, const char *s, size_t size)
{
#ifndef UNICODE
  mbstowcs(d, s, size);
#else
  MultiByteToWideChar(CP_ACP, 0, s, -1, d, (int)size);
#endif
  d[size-1]=0;
}

TCHAR *_trim(TCHAR *arg)
{
  while (*arg && (*arg==' '||*arg=='\t'||*arg=='\n'||*arg=='\r')) arg++;
  TCHAR *p=arg+_tcslen(arg)-1;
  while (p>=arg && (*p==' '||*p=='\t'||*p=='\n'||*p=='\r')) *p--=0;
  return arg;
}

TCHAR *_trimquotes(TCHAR *arg)
{
  if (*arg && (*arg=='"')) arg++;
  TCHAR *p=arg+_tcslen(arg)-1;
  if (p>=arg && (*p=='"')) *p--=0;
  return arg;
}

int _truestr(const TCHAR *s)
{
  return s && (!_tcscmp(s, _T("1")) || !_tcsicmp(s, _T("yes")) 
    || !_tcsicmp(s, _T("true")));
}

void _tooem(char *s)
{
  CharToOemBuffA(s, s, (int)strlen(s));
}

void _toansi(char *s)
{
  OemToCharBuffA(s, s, (int)strlen(s));
}

TCHAR* _tcsrstr(const TCHAR* wcs1, const TCHAR* wcs2)
{
  TCHAR *cp=(TCHAR*)_tcsend(wcs1)-1;
  TCHAR *s1, *s2;
  if (!*wcs2) return cp;
  while (cp>=wcs1)
  {
    s1=cp;
    s2=(TCHAR*) wcs2;
    while (*s1 && *s2 && !(*s1-*s2)) s1++, s2++;
    if (!*s2) return cp;
    cp--;
  }
  return NULL;
}

TCHAR* _tcsrpbrk(const TCHAR* string, const TCHAR* control)
{
  TCHAR *wcset;
  TCHAR *str=(TCHAR*)_tcsend(string)-1;
  /* 1st char in control string stops search */
  while (str>=string) 
  {
    for (wcset=(TCHAR*)control; *wcset; wcset++) 
      if (*wcset==*str) 
        return str;
    str--;
  }
  return NULL;
}

TCHAR* _tcsrspnp(const TCHAR* string, const TCHAR* control)
{
  TCHAR *str=(TCHAR*)_tcsend(string)-1;
  TCHAR *ctl;
  /* 1st char not in control string stops search */
  while (str>=string) 
  {
    for (ctl=(TCHAR*)control; *ctl!=*str; ctl++) 
      if (*ctl==(TCHAR)0) 
        /* reached end of control string without finding a match */
        return str;
    str--;
  }
  /* The whole string consisted of characters from control */
  return NULL;
}
