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

#ifndef	__PLUGIN_H__
#define	__PLUGIN_H__

#pragma once

#include "../valuelist.h"
#include "../stringlist.h"
#include "interface/plugin.hpp"
#include "interface/farcolor.hpp"
#include "dialog.h"
#include "registry.h"
#include "ui.h"
#include "progress.h"
#include "panel.h"

class FarPlugin
{
public:
	FarPlugin() : flags(0), menu(NULL), config(NULL) {}
	virtual ~FarPlugin();
	virtual void Create();
	virtual int Configure(int);
	virtual void OpenPlugin(int, int);
	void InitLang();
	virtual void LoadOptions();
	virtual void SaveOptions();
	virtual void InitOptions() {}

	virtual void FillInfo(PluginInfo* info) const;

	const char* RegRootKey() const { return "UnknownPlugin"; }

	const LocaleList& Locale() const { return locale; }
	PropertyList& Options() { return options; }
	FarDialogList& Dialogs() { return dialogs; }

protected:
	String GetDLLPath();

	FarDialogList dialogs;
	PropertyList options;
	StringList MenuItems;
	StringList ConfigItems;
	FarRegistry registry;
	String CurLocaleFile;
	LocaleList locale;

	int flags;
	mutable const wchar_t** menu;
	mutable const wchar_t** config;
};

const String& LOC(const String& l);

extern PluginStartupInfo Info;
extern FarPlugin* plugin;
extern HANDLE hInstance;

FarPlugin* CreatePlugin();

#endif//__PLUGIN_H__
