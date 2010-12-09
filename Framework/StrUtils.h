#pragma once
#include "objstring.h"

class FmtArg
{
public:
  String string;
  int size;
  union
  {
    int i;
    __int64 i64;
    double d;
    wchar_t c;
    void* p;
  } u;

  FmtArg(int v) { size=sizeof(int); u.i=v; }
  FmtArg(__int64 v) { size=sizeof(__int64); u.i64=v; }
  FmtArg(double v) { size=sizeof(double); u.d=v; }
  FmtArg(wchar_t v) { size=sizeof(wchar_t); u.c=v; }
  FmtArg(const String& v) { size=sizeof(void*); u.p=v.Lock(); string=v;}
  FmtArg(const wchar_t* v) { size=sizeof(void*); u.p=(void*)v; }
  FmtArg(const void* v) { size=sizeof(void*); u.p=(void*)v;}
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
