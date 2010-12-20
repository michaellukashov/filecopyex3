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

#ifndef	__PROPERTIES_H__
#define	__PROPERTIES_H__

#pragma once

#include "stringlist.h"
#include "array.h"

class Property
{
	enum Type { vtInt, vtFloat, vtString, vtUnknown };
public:
	Property() : Type(vtUnknown) {}
	Property(int);
	Property(float);
	Property(const String&);

	void operator=(const Property& p);

	operator int() const;
	operator bool() const;
	operator float() const;
	operator const String() const;
	bool operator== (const String&) const;
	bool operator== (int) const;
	bool operator== (float) const;
	bool operator== (const Property&) const;
	bool operator!= (const String&) const;
	bool operator!= (int) const;
	bool operator!= (float) const;
	bool operator!= (const Property&) const;
	bool operator!() const;

protected:
	Type Type;
	int Int;
	float Float;
	String Str;
};

class PropertyStore
{
public:
	Property& operator[](int i) { return Values[i]; }

	int Add(const Property& p) { return Values.Add(p); }
	int Count() const { return Values.Count(); }

	void SaveToList(StringList&, StringList&);
	void LoadFromList(StringList&, StringList&);

private:
	Array<Property> Values;
};

class PropertyList
{
public:
	Property& operator[](int i) { return store[i]; }
	Property& operator[](const String& n) { return GetValueByName(n); }

	void GetNameAndValue(int i, String& name, String& value)
	{
		name  = (String)names[i];
		value = (String)store[i];
	}

	Property& GetValueByName(const String& n)
	{
		int i=names.Find(n);
		if (i!=-1) return store[i];
		else return undef_property;
	}

	int Add(const String& n, int v) { return names.Add(n, store.Add(v)); }
	int Add(const String& n, float v) { return names.Add(n, store.Add(v)); }
	int Add(const String& n, const String& v) { return names.Add(n, store.Add(v)); }

	int Count() const { return names.Count(); }

	void SaveToList(StringList& l) { store.SaveToList(l, names); }
	void LoadFromList(StringList& l) { store.LoadFromList(l, names); }

private:
	PropertyStore store;
	StringList names;
	Property undef_property;
};

#endif//__PROPERTIES_H__
