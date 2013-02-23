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

#include <map>
#include <vector>

#include "Array.h"
#include "ObjString.h"
#include "StringList.h"

class Property
{
	enum Type { vtUnknown, vtInt, vtFloat, vtString };
public:
	Property() : type(vtUnknown) {}
	Property(int);
	Property(float);
	Property(const String&);

	void operator = (const Property& p);

	operator int() const;
	operator bool() const;
	operator float() const;
	operator const String() const;

	bool operator==(int v) const { return v == (int)*this; }
	bool operator==(bool v) const { return v == (bool)*this; }
	bool operator==(float v) const { return v == (float)*this; }
	bool operator==(const String& v) const { return v == (const String)*this; }
	bool operator==(const Property& v) const;

	bool operator!=(int v) const { return !operator==(v); }
	bool operator!=(bool v) const { return !operator==(v); }
	bool operator!=(float v) const { return !operator==(v); }
	bool operator!=(const String& v) const { return !operator==(v); }
	bool operator!=(const Property& v) const { return !operator==(v); }

	bool operator!() const { return !(bool)*this; }

protected:
	Type type;
	int vInt;
	float vFloat;
	String vStr;
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

typedef std::map<String, Property> PropertyMap;

#endif//__PROPERTIES_H__
