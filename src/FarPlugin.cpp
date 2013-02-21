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

#include "Framework/stdhdr.h"
#include "Framework/lowlevelstr.h"
#include "Framework/valuelist.h"
#include "Framework/properties.h"
#include "Framework/FileUtils.h"
#include "FarPlugin.h"
#include "version.hpp"
#include "common.h"

FarPlugin::~FarPlugin()
{
	free(menu);
	free(config);
}

void FarPlugin::InitLang()
{
	String fn = GetMsg(0);
	if (fn != CurLocaleFile)
	{
		CurLocaleFile = fn;
		locale.Load(GetDLLPath() + "\\resource\\" + fn);
	}
}

void FarPlugin::Create()
{
	// bug #15 fixed by Ivanych
	// XXX registry.root_key = ""; //XXX String(Info.RootKey) + "\\" + RegRootKey();
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
	/* 	XXX Replace registry 
	StringList temp;
	registry.ReadList("Options", temp);
	options.LoadFromList(temp);
	*/
}

void FarPlugin::SaveOptions()
{
	/* 	XXX Replace registry 
	StringList temp;
	options.SaveToList(temp);
	registry.WriteList("Options", temp);
	*/
}

int FarPlugin::Configure(int)
{
	return FALSE;
}

void FarPlugin::OpenPlugin(const struct OpenInfo *OInfo)
{
}

String FarPlugin::GetDLLPath()
{
	wchar_t buf[MAX_FILENAME];
	GetModuleFileName((HMODULE)hInstance, buf, MAX_FILENAME);
	String dlln = buf;
	return dlln.substr(0, dlln.crfind('\\'));
}
