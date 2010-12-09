#pragma once
#include "../array.h"
#include "interface/plugin.hpp"

class FarMenu
{
public:
  FarMenu(void);
  virtual ~FarMenu(void);
  void SetTitle(const String&);
  void SetBottom(const String&);
  void SetHelpTopic(const String&);
  void SetFlags(int f);
  void AddLine(const String&);
  void AddLineCheck(const String&, int check);
  void AddSep();
  void SetSelection(int n);
  int Execute();
protected:
	void SetItemText(FarMenuItem* item, const String& text);
  String Title, Bottom, HelpTopic;
  int Flags, Selection;
  Array<FarMenuItem> items;
};

int ShowMessage(const String&, const String&, int);
int ShowMessageHelp(const String&, const String&, int, const String&);
int ShowMessageEx(const String&, const String&, const String&, int);
int ShowMessageExHelp(const String&, const String&, const String&, int, const String&);

#define RES_RETRY 1
#define RES_SKIP 0

void Error(const String&, int code);
void Error2(const String&, const String&, int code);
int Error2RS(const String&, const String&, int code);

String GetErrText(int code);

String FormatWidth(const String&, int);
String FormatWidthNoExt(const String&, int);
String SplitWidth(const String&, int);

inline __int64 GetTime()
{
  LARGE_INTEGER res;
  QueryPerformanceCounter(&res);
  return res.QuadPart;
}

inline __int64 TicksPerSec()
{
  LARGE_INTEGER res;
  QueryPerformanceFrequency(&res);
  return res.QuadPart;
}

