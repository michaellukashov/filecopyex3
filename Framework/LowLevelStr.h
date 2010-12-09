#pragma once
#include <stdarg.h>

void _wtoacs(char *d, const wchar_t *s, size_t size);
void _atowcs(wchar_t *d, const char *s, size_t size);

void _tooem(char* s);
void _toansi(char* s);

TCHAR *_trim(TCHAR *s);
TCHAR *_trimquotes(TCHAR *s);

int _truestr(const TCHAR *s);

#ifdef UNICODE
#   define _atotcs(d, s, c) _atowcs(d, s, c)
#   define _wtotcs(d, s, c) _wcopy(d, s, c)
#   define _ttoacs(d, s, c) _wtoacs(d, s, c)
#   define _ttowcs(d, s, c) _wcopy(d, s, c)
#else
#   define _atotcs(d, s, c) _acopy(d, s, c)
#   define _wtotcs(d, s, c) _wtoacs(d, s, c)
#   define _ttoacs(d, s, c) _acopy(d, s, c)
#   define _ttowcs(d, s, c) _atowcs(d, s, c)
#endif

inline const TCHAR* _tcsend(const TCHAR* s) 
{
  TCHAR* res=(TCHAR*)s;
  while (*res) res++;
  return res;
}

inline const char* strend(const char* s)
{
  char* res=(char*)s;
  while (*res) res++;
  return res;
}

TCHAR* _tcsrstr(const TCHAR* wcs1, const TCHAR* wcs2);
TCHAR* _tcsrpbrk(const TCHAR* string, const TCHAR* control);
TCHAR* _tcsrspnp(const TCHAR* string, const TCHAR* control);

#if _MSC_VER <= 1200
  inline double _tstof(const TCHAR* s)
  {
    TCHAR *p;
    return _tcstod(s, &p);
  }
#endif

inline void _wcopy(wchar_t *d, const wchar_t *s, size_t size)
{
  wcsncpy(d, s, size);
  d[size-1]=0;
}

inline void _acopy(char *d, const char *s, size_t size)
{
  strncpy(d, s, size);
  d[size-1]=0;
}

inline void _tcopy(TCHAR *d, const TCHAR *s, size_t size)
{
  _tcsncpy(d, s, size);
  d[size-1]=0;
}

inline void _tcat(TCHAR *d, const TCHAR *s, size_t size)
{
  _tcsncat(d, s, size-_tcslen(d));
  d[size-1]=0;
}

