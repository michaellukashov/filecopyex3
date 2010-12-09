#pragma once
#include "objstring.h"

class FmtArg
{
public:
  Handle string;
  int size;
  union
  {
    int i;
    __int64 i64;
    double d;
    TCHAR c;
    void* p;
  } u;

  FmtArg(int v) { size=sizeof(int); u.i=v; string=InvHandle; }
  FmtArg(__int64 v) { size=sizeof(__int64); u.i64=v; string=InvHandle; }
  FmtArg(double v) { size=sizeof(double); u.d=v; string=InvHandle; }
  FmtArg(TCHAR v) { size=sizeof(TCHAR); u.c=v; string=InvHandle; }
  FmtArg(const String& v) { size=sizeof(void*); u.p=v.Lock(); string=v.handle();}
  FmtArg(const TCHAR* v) { size=sizeof(void*); u.p=(void*)v; string=InvHandle; }
  FmtArg(const void* v) { size=sizeof(void*); u.p=(void*)v; string=InvHandle; }

  ~FmtArg() { if (string!=InvHandle) heap->Unlock(string); }
};

String Format(const String& fmt,  
              const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, 
              const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, 
              const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, 
              const FmtArg& =0, const FmtArg& =0, const FmtArg& =0, const FmtArg& =0);

String FormatNum(__int64);
String FormatTime(const FILETIME&);

String Replace(const String&, const String&, const String&);

String FormatProgress(__int64 cb, __int64 total);
String FormatSpeed(__int64 cb);
String FormatValue(__int64 Value);
