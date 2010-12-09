#include <stdhdr.h>
#include "objstring.h"

const String String::substr(int s, int l) const
{
	if (s>=len() || s<0 || l<=0) return String();
	if (l>len()) l=len();
	return str.substr(s, l);
}

const String String::trim() const
{
  wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
  wchar_t *p=Lock();
  _tcscpy_s(sp, len() + 1, p);
  wchar_t *ep=sp+len()-1;
  // bug #46 fixed by axxie
  while (*sp && isbadchar(*sp)) sp++;
  while (ep>=sp && isbadchar(*ep)) ep--;
  *(ep+1)=0;
  Unlock();
  return String(sp);
}

const String String::ltrim() const
{
  wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
  wchar_t *p=Lock();
  _tcscpy_s(sp, len() + 1, p);
  // bug #46 fixed by axxie
  while (*sp && isbadchar(*sp)) sp++;
  Unlock();
  return String(sp);
}

const String String::rtrim() const
{
  wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
  wchar_t *p=Lock();
  _tcscpy_s(sp, len() + 1, p);
  wchar_t *ep=sp+len()-1;
  // bug #46 fixed by axxie
  while (ep>=sp && isbadchar(*ep)) ep--;
  *(ep+1)=0;
  Unlock();
  return String(sp);
}

const String String::trimquotes() const
{
  wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
  wchar_t *p=Lock();
  _tcscpy_s(sp, len() + 1, p);
  wchar_t *ep=sp+len()-1;
  if (*sp && *sp=='"') sp++;
  if (ep>=sp && *ep=='"') *ep=0;
  Unlock();
  return String(sp);
}

const String String::rev() const
{
  wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
  wchar_t *p=Lock();
  wchar_t *ep=sp+len()-1;
  *(ep+1)=0;
  while (*p) *ep--=*p++;
  Unlock();
  return String(sp);
}

/*
Handle Heap::AllocString(const String& str)
{
	size_t l = str.len()+1;
  Handle res=Alloc((int)l*sizeof(wchar_t));
  _tcscpy_s(LockString(res), l, str.Lock());
  Unlock(res);
  str.Unlock();
  return res;
}

void Heap::ReallocString(Handle hnd, const String& str)
{
	size_t l = str.len()+1;
  Realloc(hnd, (int)l*sizeof(wchar_t));
  _tcscpy_s(LockString(hnd), l, str.Lock());
  Unlock(hnd);
  str.Unlock();
}
*/

const String String::replace(const String &what, const String &with,
                             int nocase) const
{
  if (!what.len()) return *this;
  String res;
  int start=0, p;
  if (!nocase)
  {
    while (p=find(what, start),p!=-1)
    {
      res+=substr(start, p-start);
      res+=with;
      start=p+with.len();
    }
  }
  else
  {
    String what1=what.toUpper(), this1=toUpper();
    while (p=this1.find(what1, start),p!=-1)
    {
      res+=substr(start, p-start);
      res+=with;
      start=p+with.len();
    }
  }
  res+=substr(start);
  return res;
}

const String String::toUpper() const
{
  wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
  wchar_t *p=Lock();
  _tcscpy_s(sp, len()+1, p);
  CharUpperBuff(sp, len());
  Unlock();
  return String(sp);
}

const String String::toLower() const
{
  wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
  wchar_t *p=Lock();
  _tcscpy_s(sp, len() + 1, p);
  CharLowerBuff(sp, len());
  Unlock();
  return String(sp);
}


