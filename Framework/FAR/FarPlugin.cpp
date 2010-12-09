#include <stdhdr.h>
#include "lowlevelstr.h"
#include "fwcommon.h"
#include "valuelist.h"
#include "properties.h"
#include "far/farplugin.h"

PluginStartupInfo Info;
String PluginRootKey;
FarPlugin *Instance=NULL;

String CurLocaleFile;
LocaleList *Locale;

void FarPlugin::InitLang()
{
  String fn=Info.GetMsg(Info.ModuleNumber, 0);
  if (fn!=CurLocaleFile)
  {
    Locale->Load(GetDLLPath()+"\\resource\\"+fn);
    CurLocaleFile=fn;
  }
}

FarPlugin::FarPlugin(void)
{
  // bug #15 fixed by Ivanych
  SetFileApisToOEM();
  Prefix="";
  RootKey="UnknownPlugin";
  Flags=0;

  InitLang();

  if (!Dialogs.Load(GetDLLPath()+"\\resource\\dialogs.objd"))
  {
    FWError("Could not load DIALOGS.OBJD");
    exit(0);
  }
}

FarPlugin::~FarPlugin(void)
{
}

void FarPlugin::LoadOptions()
{
  StringList temp;
  Registry.ReadList("Options", temp);
  Options.LoadFromList(temp);
}

void FarPlugin::SaveOptions()
{
  StringList temp;
  Options.SaveToList(temp);
  Registry.WriteList("Options", temp);
}

void FarPlugin::InitOptions(PropertyList&)
{
}

void Init()
{
  Locale=new LocaleList();
  errorHandler=FarErrorHandler;
  __FarDlgObjectReg=new FarDlgObjectReg;
  Instance=InitInstance();
  PluginRootKey=String(::Info.RootKey)+"\\"+Instance->RootKey;
  Instance->InitOptions(Instance->Options);
  Instance->LoadOptions();
}

void _export WINAPI SetStartupInfo(const struct PluginStartupInfo *Info)
{
  SetFileApisToANSI();
  ::Info=*Info;
  if (!Instance) Init();
  SetFileApisToOEM();
}

HANDLE _export WINAPI OpenPlugin(int OpenFrom, int Item)
{
  SetFileApisToANSI();
  Instance->InitLang();
  FarPanel* p=Instance->OpenPlugin(OpenFrom, Item);
  if (!p) Instance->SaveOptions();
  SetFileApisToOEM();
  if (p) return (HANDLE)p;
  else return INVALID_HANDLE_VALUE;
}

int _export WINAPI Configure(int ItemNumber)
{
  SetFileApisToANSI();
  Instance->InitLang();
  int res=Instance->Configure(ItemNumber);
  Instance->SaveOptions();
  SetFileApisToOEM();
  return res;
}

char **menu=NULL, **config=NULL, *prefix=NULL;

void _export WINAPI GetPluginInfo(struct PluginInfo *Info)
{
  SetFileApisToANSI();
  Instance->InitLang();
  memset(Info, 0, sizeof(struct PluginInfo));
  Info->StructSize=sizeof(struct PluginInfo);
  Info->Flags=Instance->Flags;
  if (!prefix && Instance->Prefix!="") prefix=strdup(Instance->Prefix.aptr());
  Info->CommandPrefix=prefix;
  Info->DiskMenuStringsNumber=0;
  Info->DiskMenuNumbers=NULL;
  if (!menu)
  {
    menu=(char**)malloc(sizeof(char*)*Instance->MenuItems.Count());
    for (int i=0; i<Instance->MenuItems.Count(); i++)
      menu[i]=strdup(Instance->MenuItems[i].optr());
  }
  if (!config)
  {
    config=(char**)malloc(sizeof(char*)*Instance->ConfigItems.Count());
    for (int i=0; i<Instance->ConfigItems.Count(); i++)
      config[i]=strdup(Instance->ConfigItems[i].optr());
  }
  Info->PluginMenuStringsNumber=Instance->MenuItems.Count();
  Info->PluginMenuStrings=menu;
  Info->PluginConfigStringsNumber=Instance->ConfigItems.Count();
  Info->PluginConfigStrings=config;
  SetFileApisToOEM();
}

void _export WINAPI ClosePlugin(HANDLE hPlugin)
{
  SetFileApisToANSI();
  Instance->SaveOptions();
  delete (FarPanel*)hPlugin;
  SetFileApisToOEM();
}

int _export WINAPI GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1282);
}

int FarPlugin::Configure(int)
{
  return FALSE;
}

FarPanel* FarPlugin::OpenPlugin(int, int)
{
  return NULL;
}


extern HANDLE hInstance;

String GetDLLName()
{
  TCHAR buf[1024];
  GetModuleFileName((HMODULE)hInstance, buf, 1024);
  return buf;
}

String GetDLLPath()
{
  String dlln=GetDLLName();
  return dlln.substr(0, dlln.crfind('\\'));
}

void FarErrorHandler(const TCHAR* s)
{
  /*if (ShowMessageEx("Error", s, "OK\nDebug", 0)==1)
    DebugBreak();*/
  char text[1024];
  _ttoacs(text, s, 1024);
  const char* items[]={
    "Framework Error", text, "OK", "Debug"
  };
  if (Info.Message(Info.ModuleNumber, FMSG_WARNING, NULL, items, 4, 2)==1)
    DebugBreak();
}
