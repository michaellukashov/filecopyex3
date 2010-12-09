#include <stdhdr.h>
#include "objstring.h"

const String String::substr(int s, int l) const
{
  if (s>=_len || s<0 || l<=0) return String();
  if (l>_len) l=_len;
  TCHAR *op=(TCHAR*)_alloca((l+1)*sizeof(TCHAR)), *sp=op;
  TCHAR *p=Lock();
  TCHAR *ptr=p+s;
  int c=0;
  while (*ptr && c<l) 
  {
    *sp++=*ptr++;
    c++;
  }
  *sp++=0;
  Unlock();
  return String(op);
}

const String String::trim() const
{
  TCHAR *sp=(TCHAR*)_alloca((_len+1)*sizeof(TCHAR));
  TCHAR *p=Lock();
  _tcscpy(sp, p);
  TCHAR *ep=sp+_len-1;
  // bug #46 fixed by axxie
  while (*sp && isbadchar(*sp)) sp++;
  while (ep>=sp && isbadchar(*ep)) ep--;
  *(ep+1)=0;
  Unlock();
  return String(sp);
}

const String String::ltrim() const
{
  TCHAR *sp=(TCHAR*)_alloca((_len+1)*sizeof(TCHAR));
  TCHAR *p=Lock();
  _tcscpy(sp, p);
  // bug #46 fixed by axxie
  while (*sp && isbadchar(*sp)) sp++;
  Unlock();
  return String(sp);
}

const String String::rtrim() const
{
  TCHAR *sp=(TCHAR*)_alloca((_len+1)*sizeof(TCHAR));
  TCHAR *p=Lock();
  _tcscpy(sp, p);
  TCHAR *ep=sp+_len-1;
  // bug #46 fixed by axxie
  while (ep>=sp && isbadchar(*ep)) ep--;
  *(ep+1)=0;
  Unlock();
  return String(sp);
}

const String String::trimquotes() const
{
  TCHAR *sp=(TCHAR*)_alloca((_len+1)*sizeof(TCHAR));
  TCHAR *p=Lock();
  _tcscpy(sp, p);
  TCHAR *ep=sp+_len-1;
  if (*sp && *sp=='"') sp++;
  if (ep>=sp && *ep=='"') *ep=0;
  Unlock();
  return String(sp);
}

const String String::rev() const
{
  TCHAR *sp=(TCHAR*)_alloca((_len+1)*sizeof(TCHAR));
  TCHAR *p=Lock();
  TCHAR *ep=sp+_len-1;
  *(ep+1)=0;
  while (*p) *ep--=*p++;
  Unlock();
  return String(sp);
}

Handle Heap::AllocString(const String& str)
{
  Handle res=Alloc((str.len()+1)*sizeof(TCHAR));
  _tcscpy(LockString(res), str.Lock());
  Unlock(res);
  str.Unlock();
  return res;
}

void Heap::ReallocString(Handle hnd, const String& str)
{
  Realloc(hnd, (str.len()+1)*sizeof(TCHAR));
  _tcscpy(LockString(hnd), str.Lock());
  Unlock(hnd);
  str.Unlock();
}

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
  TCHAR *sp=(TCHAR*)_alloca((_len+1)*sizeof(TCHAR));
  TCHAR *p=Lock();
  _tcscpy(sp, p);
  CharUpperBuff(sp, len());
  Unlock();
  return String(sp);
}

const String String::toLower() const
{
  TCHAR *sp=(TCHAR*)_alloca((_len+1)*sizeof(TCHAR));
  TCHAR *p=Lock();
  _tcscpy(sp, p);
  CharLowerBuff(sp, len());
  Unlock();
  return String(sp);
}


