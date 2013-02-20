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
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"

PluginStartupInfo Info;
FarPlugin* plugin = NULL;

FarPlugin::~FarPlugin()
{
	free(menu);
	free(config);
}

void FarPlugin::InitLang()
{
	String fn = ""; //XXX Info.GetMsg(Info.ModuleNumber, 0);
	if (fn != CurLocaleFile)
	{
		CurLocaleFile = fn;
		locale.Load(GetDLLPath() + "\\resource\\" + fn);
	}
}

void FarPlugin::Create()
{
	// bug #15 fixed by Ivanych
	registry.root_key = ""; //XXX String(Info.RootKey) + "\\" + RegRootKey();
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
	//XXX info->DiskMenuStringsNumber = 0;
	//XXX info->DiskMenuNumbers = NULL;
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
	//XXX info->PluginMenuStringsNumber=plugin->MenuItems.Count();
	//XXX info->PluginMenuStrings=menu;
	//XXX info->PluginConfigStringsNumber=plugin->ConfigItems.Count();
	//XXX info->PluginConfigStrings=config;
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

static void FarErrorHandler(const wchar_t* s)
{
	/*if (ShowMessageEx("Error", s, "OK\nDebug", 0)==1)
	DebugBreak();*/
	const wchar_t* items[]={ L"Framework Error", s, L"OK", L"Debug" };
	//XXX if (Info.Message(Info.ModuleNumber, FMSG_WARNING, NULL, items, 4, 2)==1)
    //XXX 		DebugBreak();
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
	Info = *psi;
	if(!plugin)
	{
		errorHandler = FarErrorHandler;
		InitObjMgr();
		plugin = CreatePlugin();
	}
}

HANDLE WINAPI OpenW(int OpenFrom, INT_PTR Item)
{
	plugin->InitLang();
	plugin->OpenPlugin(OpenFrom, (int)Item);
	plugin->SaveOptions();
	//  if (p) return (HANDLE)p;
	//  else
	return INVALID_HANDLE_VALUE;
}

int WINAPI ConfigureW(int ItemNumber)
{
	plugin->InitLang();
	int res=plugin->Configure(ItemNumber);
	plugin->SaveOptions();
	return res;
}

/*
 Функция GetMsg возвращает строку сообщения из языкового файла.
 А это надстройка над Info.GetMsg для сокращения кода :-)
*/
const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

/*
Функция GetPluginInfoW вызывается для получения информации о плагине
*/
void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize = sizeof(*Info);
	Info->Flags = PF_EDITOR;
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0] = L"FileCopyEx"; //GetMsg(MTitle);
	Info->PluginMenu.Guids = &MenuGuid;
	Info->PluginMenu.Strings = PluginMenuStrings;
	Info->PluginMenu.Count = ARRAYSIZE(PluginMenuStrings);

	plugin->InitLang();
	plugin->FillInfo(Info);
}

void WINAPI ClosePluginW(HANDLE hPlugin)
{
	plugin->SaveOptions();
	//  delete (FarPanel*)hPlugin;
}

void WINAPI ExitFARW()
{
	delete plugin;
	DoneObjMgr();
};

const String& LOC(const String& l)
{
	return plugin->Locale()[l];
}

int WINAPI GetMinFarVersionW(void)
{
	return 0; //XXX FARMANAGERVERSION;
}

int FarPlugin::Configure(int)
{
	return FALSE;
}

void FarPlugin::OpenPlugin(int, int)
{
}

String FarPlugin::GetDLLPath()
{
	wchar_t buf[MAX_FILENAME];
	GetModuleFileName((HMODULE)hInstance, buf, MAX_FILENAME);
	String dlln = buf;
	return dlln.substr(0, dlln.crfind('\\'));
}
