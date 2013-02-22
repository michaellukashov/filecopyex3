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

#ifndef	__FARPLUGIN_H__
#define	__FARPLUGIN_H__

#pragma once

#include "Framework/ObjString.h"
#include "Framework/Properties.h"
#include "Framework/ValueList.h"
#include "dlgobject.h"
#include "registry.h"

class FarPlugin
{
public:
	FarPlugin() : flags(0), menu(NULL), config(NULL) {}
	~FarPlugin();
	void Create();
	int Configure(int);
	void OpenPlugin(const struct OpenInfo *OInfo);
	void InitLang();
	void LoadOptions();
	void SaveOptions();
	void InitOptions();
	void Config();

	void FillInfo(PluginInfo* info) const;

	const LocaleList& Locale() const { return locale; }
	PropertyList& Options() { return options; }
	FarDialogList& Dialogs() { return dialogs; }

	virtual const char* RegRootKey() const { return "FileCopyEx"; }

	const StringList& Descs() const { return descs; }
	// void	MacroCommand(const FARMACROCOMMAND& cmd); // XXX???

private:
	String GetDLLPath();

	FarDialogList dialogs;
	PropertyList options;
	StringList MenuItems;
	StringList ConfigItems;
	FarRegistry registry;
	String CurLocaleFile;
	LocaleList locale;
	StringList descs;

	int flags;
	mutable const wchar_t** menu;
	mutable const wchar_t** config;

	void	About();
	void	KeyConfig();

	void	Bind(const String& key, const String& seq);
	void	Unbind(const String& key);
	int		Binded(const String &key);


};

// FileCopyExPlugin* Plugin();

#endif//__PLUGIN_H__
