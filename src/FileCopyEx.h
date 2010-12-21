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

#ifndef	__FILECOPYEX_H__
#define	__FILECOPYEX_H__

#pragma once

#include "../framework/properties.h"
#include "../framework/far/plugin.h"

#define MRES_NONE         0
#define MRES_STDCOPY      1
#define MRES_STDCOPY_RET  2

class FileCopyExPlugin : public FarPlugin
{
public:
	virtual ~FileCopyExPlugin(void);
	virtual void Create();
	virtual int Configure(int);
	virtual void OpenPlugin(int, int);
	virtual void InitOptions();
	void Config();
	virtual const char* RegRootKey() const { return "FileCopyEx"; }

	const StringList& Descs() const { return descs; }

private:
	void About();
	void KeyConfig();

	void reloadmacro();
	int isour(const String &key);
	void restore(const String &key);
	void dobind(const String& key, const String& seq);

private:
	StringList descs;
};

FileCopyExPlugin* Plugin();

#endif//__FILECOPYEX_H__
