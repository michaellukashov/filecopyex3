/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010
Idea & core: Max Antipin
Coding: Serge Cheperis aka craZZy
Bugfixes: slst, CDK, Ivanych, Alter, Axxie and Nsky
Special thanks to Vitaliy Tsubin
Far 2 (32 & 64 bit) full unicode version by djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../stdhdr.h"
#include "../lowlevelstr.h"
#include "../common.h"
#include "../valuelist.h"
#include "../properties.h"
#include "plugin.h"

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
//  SetFileApisToOEM();
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

void _export WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
//  SetFileApisToANSI();
  ::Info=*Info;
  if(!Instance)
	  Init();
//  SetFileApisToOEM();
}

HANDLE _export WINAPI OpenPluginW(int OpenFrom, INT_PTR Item)
{
//  SetFileApisToANSI();
  Instance->InitLang();
  FarPanel* p=Instance->OpenPlugin(OpenFrom, (int)Item);
  if (!p) Instance->SaveOptions();
//  SetFileApisToOEM();
  if (p) return (HANDLE)p;
  else return INVALID_HANDLE_VALUE;
}

int _export WINAPI ConfigureW(int ItemNumber)
{
//  SetFileApisToANSI();
  Instance->InitLang();
  int res=Instance->Configure(ItemNumber);
  Instance->SaveOptions();
//  SetFileApisToOEM();
  return res;
}

wchar_t **menu=NULL, **config=NULL, *prefix=NULL;

void _export WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
//  SetFileApisToANSI();
  Instance->InitLang();
  memset(Info, 0, sizeof(struct PluginInfo));
  Info->StructSize=sizeof(struct PluginInfo);
  Info->Flags=Instance->Flags;
  if (!prefix && Instance->Prefix!="") prefix=_wcsdup(Instance->Prefix.ptr());
  Info->CommandPrefix=prefix;
  Info->DiskMenuStringsNumber=0;
  Info->DiskMenuNumbers=NULL;
  if (!menu)
  {
    menu=(wchar_t**)malloc(sizeof(wchar_t*)*Instance->MenuItems.Count());
    for (int i=0; i<Instance->MenuItems.Count(); i++)
      menu[i]=_wcsdup(Instance->MenuItems[i].ptr());
  }
  if (!config)
  {
    config=(wchar_t**)malloc(sizeof(wchar_t*)*Instance->ConfigItems.Count());
    for (int i=0; i<Instance->ConfigItems.Count(); i++)
      config[i]=_wcsdup(Instance->ConfigItems[i].ptr());
  }
  Info->PluginMenuStringsNumber=Instance->MenuItems.Count();
  Info->PluginMenuStrings=menu;
  Info->PluginConfigStringsNumber=Instance->ConfigItems.Count();
  Info->PluginConfigStrings=config;
//  SetFileApisToOEM();
}

void _export WINAPI ClosePluginW(HANDLE hPlugin)
{
//  SetFileApisToANSI();
  Instance->SaveOptions();
  delete (FarPanel*)hPlugin;
//  SetFileApisToOEM();
}

int _export WINAPI GetMinFarVersionW(void)
{
  return FARMANAGERVERSION;
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
  wchar_t buf[1024];
  GetModuleFileName((HMODULE)hInstance, buf, 1024);
  return buf;
}

String GetDLLPath()
{
  String dlln=GetDLLName();
  return dlln.substr(0, dlln.crfind('\\'));
}

void FarErrorHandler(const wchar_t* s)
{
  /*if (ShowMessageEx("Error", s, "OK\nDebug", 0)==1)
    DebugBreak();*/
  const wchar_t* items[]={ L"Framework Error", s, L"OK", L"Debug" };
  if (Info.Message(Info.ModuleNumber, FMSG_WARNING, NULL, items, 4, 2)==1)
    DebugBreak();
}
