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
FarPlugin* plugin = NULL;

FarPlugin::~FarPlugin()
{
	free(menu);
	free(config);
}

void FarPlugin::InitLang()
{
	String fn = Info.GetMsg(Info.ModuleNumber, 0);
	if (fn != CurLocaleFile)
	{
		CurLocaleFile = fn;
		locale.Load(GetDLLPath() + "\\resource\\" + fn);
	}
}

void FarPlugin::Create()
{
	// bug #15 fixed by Ivanych
	registry.root_key = String(Info.RootKey) + "\\" + RegRootKey();
	InitLang();

	if(!dialogs.Load(GetDLLPath() + "\\resource\\dialogs.objd"))
	{
		FWError("Could not load dialogs.objd");
		exit(0);
	}
	InitOptions();
	LoadOptions();
}
void FarPlugin::FillInfo(PluginInfo* info) const
{
	info->Flags = flags;
	info->CommandPrefix = NULL;
	info->DiskMenuStringsNumber = 0;
	info->DiskMenuNumbers = NULL;
	if(!menu)
	{
		menu = (const wchar_t**)malloc(sizeof(const wchar_t*)*MenuItems.Count());
		for(int i = 0; i < MenuItems.Count(); ++i)
			menu[i] = MenuItems[i].ptr();
	}
	if(!config)
	{
		config = (const wchar_t**)malloc(sizeof(const wchar_t*)*ConfigItems.Count());
		for(int i = 0; i < ConfigItems.Count(); ++i)
			config[i] = ConfigItems[i].ptr();
	}
	info->PluginMenuStringsNumber=plugin->MenuItems.Count();
	info->PluginMenuStrings=menu;
	info->PluginConfigStringsNumber=plugin->ConfigItems.Count();
	info->PluginConfigStrings=config;
}

void FarPlugin::LoadOptions()
{
	StringList temp;
	registry.ReadList("Options", temp);
	options.LoadFromList(temp);
}

void FarPlugin::SaveOptions()
{
	StringList temp;
	options.SaveToList(temp);
	registry.WriteList("Options", temp);
}

void _export WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	::Info = *Info;
	if(!plugin)
	{
		errorHandler = FarErrorHandler;
		InitObjMgr();
		plugin = CreatePlugin();
	}
}

HANDLE _export WINAPI OpenPluginW(int OpenFrom, INT_PTR Item)
{
	plugin->InitLang();
	plugin->OpenPlugin(OpenFrom, (int)Item);
	plugin->SaveOptions();
	//  if (p) return (HANDLE)p;
	//  else
	return INVALID_HANDLE_VALUE;
}

int _export WINAPI ConfigureW(int ItemNumber)
{
	plugin->InitLang();
	int res=plugin->Configure(ItemNumber);
	plugin->SaveOptions();
	return res;
}

void _export WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(struct PluginInfo);
	plugin->InitLang();
	plugin->FillInfo(Info);
}

void _export WINAPI ClosePluginW(HANDLE hPlugin)
{
	plugin->SaveOptions();
	//  delete (FarPanel*)hPlugin;
}

void _export WINAPI ExitFARW()
{
	delete plugin;
	DoneObjMgr();
};

const String& LOC(const String& l)
{
	return plugin->Locale()[l];
}

int _export WINAPI GetMinFarVersionW(void)
{
	return FARMANAGERVERSION;
}

int FarPlugin::Configure(int)
{
	return FALSE;
}

void FarPlugin::OpenPlugin(int, int)
{
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
