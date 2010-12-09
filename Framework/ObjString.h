#pragma once
#include "heap.h"
#include "lowlevelstr.h"

class String;
class Heap;

class StrPtr
{
public:
  operator TCHAR*() { return ptr; }
  ~StrPtr() { heap->Unlock(hnd); }
private:
  StrPtr(Handle h) 
  { 
    hnd=h; 
    ptr=(TCHAR*)heap->Lock(h); 
  }
  Handle hnd;
  TCHAR* ptr;
  friend class String;
};

class AnsiPtr
{
public:
  operator const char*() { return ptr; }
  ~AnsiPtr() { heap->Free(hnd); }
private:
  AnsiPtr(const String& v, int oem);
  Handle hnd;
  const char* ptr;
  friend class String;
};

class String
{
public:
  String(void) 
  { 
    __Init(_T("")); 

  }
  String(const TCHAR* v) 
  {
    __Init(v); 
  }

#ifdef UNICODE
  String(const char* v)
  {
    InitHeap();
    _len=v?(int)strlen(v):0;
    hnd=heap->Alloc((_len+1)*sizeof(TCHAR));
    if (v) _atotcs(ptr(), v, _len+1);
    ownhnd=1;
  }
#endif
  String(TCHAR v, int len)
  {
    InitHeap();
    _len=len;
    hnd=heap->Alloc((_len+1)*sizeof(TCHAR));
    TCHAR *p=Lock();
    for (int i=0; i<len; i++) p[i]=v;
    p[len]=0;
    ownhnd=1;
    Unlock();
  }
  String(const String& v)
  {
    InitHeap();
    hnd=heap->AllocString(v);
    _len=v.len();
    ownhnd=1;
  }
  explicit String(Handle hnd)
  {
    this->hnd=hnd;
    _len=heap->GetSize(hnd)/sizeof(TCHAR)-1;
    ownhnd=0;
#ifdef _DEBUG
    const void* ptr=DebugPtr();
#endif
  }
  explicit String(Handle hnd, int own)
  {
    this->hnd=hnd;
    _len=heap->GetSize(hnd)/sizeof(TCHAR)-1;
    ownhnd=own;
  }

  ~String(void) { if (ownhnd) heap->Free(hnd); }

  void operator=(const String& v)
  {
    heap->ReallocString(hnd, v);
    _len=heap->GetSize(hnd)/sizeof(TCHAR)-1;
  }
  inline bool operator==(const String& v) const { return cmp(v)==0; }
  inline bool operator!=(const String& v) const { return cmp(v)!=0; }
  inline bool operator<(const String& v) const { return cmp(v)<0; }
  inline bool operator>(const String& v) const { return cmp(v)>0; }
  inline bool operator<=(const String& v) const { return cmp(v)<=0; }
  inline bool operator>=(const String& v) const { return cmp(v)>=0; }
  inline void operator+=(const String& v)
  {
    _len+=v.len();
    heap->Realloc(hnd, (_len+1)*sizeof(TCHAR));
    _tcscat(ptr(), v.ptr());
  }
  const String operator+(const String& v) const
  {
    Handle hnd=heap->Alloc((len()+v.len()+1)*sizeof(TCHAR));
    TCHAR *sp=heap->LockString(hnd);
    _tcscpy(sp, ptr());
    _tcscat(sp, v.ptr());
    heap->Unlock(hnd);
    return String(hnd, 1);
  }

  inline int len() const { return _len; }

  inline TCHAR operator[] (int i) const
  {
    if (i>=0 && i<_len) return ((const TCHAR*)ptr())[i];
    else return 0;
  }

  int AsInt() const { return _ttoi(ptr()); }
  double AsFloat() const { return _tstof(ptr()); }
  bool AsBool() const { return operator==(_T("1")); } 
  void ToAnsi(char* buf, int sz) const { _ttoacs(buf, ptr(), sz); }
  void ToOem(char* buf, int sz) const { _ttoacs(buf, ptr(), sz); _tooem(buf); }
  void ToUnicode(wchar_t* buf, int sz) const { _ttowcs(buf, ptr(), sz); }
  void CopyTo(TCHAR* buf, int sz) const { _tcopy(buf, ptr(), sz); }

  explicit String(int v)
  {
    TCHAR buf[64];
    _itot(v, buf, 10);
    __Init(buf);
  }
  explicit String(__int64 v)
  {
    TCHAR buf[64];
    _i64tot(v, buf, 10);
    __Init(buf);
  }
  explicit String(double v)
  {
    TCHAR buf[64];
    _stprintf(buf, _T("%g"), v);
    __Init(buf);
  }
  explicit String(bool v)
  {
    if (v) __Init(_T("1"));
    else __Init(_T("0"));
  }

  int cmp(const String& v) const { return ncmp(v, 0x7FFFFFFF); }
  int icmp(const String& v) const { return nicmp(v, 0x7FFFFFFF); }
  int ncmp(const String& v, int sz) const { return _tcsncmp(ptr(), v.ptr(), sz); }
  int nicmp(const String& v, int sz) const { return _tcsnicmp(ptr(), v.ptr(), sz); }
  // bug #46 fixed by axxie
  const bool isbadchar(TCHAR c) const { return c >= _T('\0') && c <= _T(' '); }

  const String substr(int, int=0x7FFFFFFF) const;
  const String left(int n) const { return substr(0, n); }
  const String right(int n) const { return substr(_len-n, n); }

  const String trim() const;
  const String ltrim() const;
  const String rtrim() const;
  const String trimquotes() const;

  const String rev() const;

  const String replace(const String& what, const String& with, int nocase=0) const;
  const String toUpper() const;
  const String toLower() const;
  
  int find(const String& v, int start=0) const
  {
    TCHAR* p=ptr(), *rp=_tcsstr(p+start, v.ptr());
    if (!rp) return -1; 
    else return (int)(rp-p);
  }
  int rfind(const String& v) const
  {
    TCHAR* p=ptr(), *rp=_tcsrstr(p, v.ptr());
    if (!rp) return -1; 
    else return (int)(rp-p);
  }
  int cfind(TCHAR v, int start=0) const
  {
    TCHAR* p=ptr(), *rp=_tcschr(p+start, v);
    if (!rp) return -1; 
    else return (int)(rp-p);
  }
  int crfind(TCHAR v) const
  {
    TCHAR* p=ptr(), *rp=_tcsrchr(p, v);
    if (!rp) return -1; 
    else return (int)(rp-p);
  }
  int cfind(const String& v, int start=0) const
  {
    TCHAR* p=ptr(), *rp=_tcspbrk(p+start, v.ptr());
    if (!rp) return -1; 
    else return (int)(rp-p);
  }
  int cnfind(const String& v, int start=0) const
  {
    TCHAR* p=ptr(), *rp=_tcsspnp(p+start, v.ptr());
    if (!rp) return -1; 
    else return (int)(rp-p);
  }
  int crfind(const String& v) const 
  { 
    TCHAR* p=ptr(), *rp=_tcsrpbrk(p, v.ptr());
    if (!rp) return -1; 
    else return (int)(rp-p);
  }
  int cnrfind(const String& v) const 
  { 
    TCHAR* p=ptr(), *rp=_tcsrspnp(p, v.ptr());
    if (!rp) return -1; 
    else return (int)(rp-p);
  }

  StrPtr ptr() const { return StrPtr(hnd); }
  AnsiPtr aptr() const { return AnsiPtr(*this, 0); }
  AnsiPtr optr() const { return AnsiPtr(*this, 1); }
  Handle handle() const { return hnd; }
  TCHAR* Lock() const { return (TCHAR*)heap->Lock(hnd); }
  void Unlock() const { heap->Unlock(hnd); }

#ifdef _DEBUG
  const TCHAR* DebugPtr() { 
      if (_len != 0xCCCCCCCC) {
          return (TCHAR*)heap->DebugPtr(hnd);
      }
      return NULL;
  }
#endif

private:
  int _len;
  Handle hnd;
  int ownhnd;
  void __Init(const TCHAR* v)
  {
    InitHeap();
    hnd=heap->AllocString(v);
    _len=heap->GetSize(hnd)/sizeof(TCHAR)-1;
    ownhnd=1;
  }

  friend class Heap;
};

inline AnsiPtr::AnsiPtr(const String& v, int oem)
{
  hnd=heap->Alloc((v.len()+1)*sizeof(TCHAR));
  ptr=(const char*)heap->Lock(hnd);
  if (!oem)
    v.ToAnsi((char*)ptr, heap->GetSize(hnd));
  else
    v.ToOem((char*)ptr, heap->GetSize(hnd));
  heap->Unlock(hnd);
}

inline String operator+ (const TCHAR* s1, const String& s2)
  { return String(s1)+s2; }
#ifdef UNICODE
  inline String operator+ (const char* s1, const String& s2)
    { return String(s1)+s2; }
#endif
