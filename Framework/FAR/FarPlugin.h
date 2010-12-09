#pragma once
#include "valuelist.h"
#include "stringlist.h"
#include "far/interface/plugin.hpp"
#include "far/interface/farcolor.hpp"
#include "far/fardialog.h"
#include "far/farregistry.h"
#include "far/farui.h"
#include "far/farprogress.h"
#include "far/farpanel.h"

class FarPlugin
{
public:
  FarPlugin(void);
  virtual ~FarPlugin(void);
  virtual int Configure(int);
  virtual FarPanel* OpenPlugin(int, int);
  StringList MenuItems, ConfigItems;
  String Prefix, RootKey;
  int Flags;
  FarDialogList Dialogs;
  void InitLang();
  PropertyList Options;
  virtual void LoadOptions();
  virtual void SaveOptions();
  virtual void InitOptions(PropertyList&);
};

extern LocaleList* Locale;
#define LOC(s) ((*Locale)[s])

extern PluginStartupInfo Info;
extern String PluginRootKey;
extern FarPlugin *Instance;
extern HANDLE hInstance;

String GetDLLName();
String GetDLLPath();

FarPlugin* InitInstance();

void FarErrorHandler(const wchar_t*);
