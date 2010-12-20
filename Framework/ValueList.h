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

#ifndef	__VALUELIST_H__
#define	__VALUELIST_H__

#pragma once

#include "stringlist.h"

class ValueList
{
public:
	const String& Name(int) const;
	const String& Value(int) const;
	int Count() const;
	void Clear();

	void Set(const String&, const String&);
	void Set(const String&);

	const String& operator[](const String&) const;

	void LoadFromList(StringList&);
	void SaveToList(StringList&);

	struct ListItem
	{
		String name;
		String value;
	};

private:
	int Find(const String& k) const;
	Array<ListItem> items;
	String empty;
};

class LocaleList : public ValueList
{
public:
	void Load(const String& fn);
};

#endif//__VALUELIST_H__
