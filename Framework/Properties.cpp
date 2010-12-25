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

#include "stdhdr.h"
#include "lowlevelstr.h"
#include "properties.h"
#include "valuelist.h"
#include "common.h"

Property::Property(int v)
{
	Type = vtInt;
	Int = v;
}

Property::Property(float v)
{
	Type = vtFloat;
	Float = v;
}

Property::Property(const String& v)
{
	Type = vtString;
	Str = v;
}

void Property::operator=(const Property& p)
{
	switch(Type)
	{
	case vtInt:		Int = (int)p;
	case vtFloat:	Float = (float)p;
	case vtString:	Str = (const String)p;
	}
}

Property::operator int() const
{
	switch(Type)
	{
	case vtInt: return Int;
	case vtFloat: return (int)Float;
	case vtString: return Str.AsInt();
	}
	return 0;
}

Property::operator bool() const
{
	switch(Type)
	{
	case vtInt: return Int != 0;
	case vtFloat: return Float != 0;
	case vtString: return Str == L"1";
	}
	return false;
}

Property::operator float() const
{
	switch(Type)
	{
	case vtInt: return (float)Int;
	case vtFloat: return Float;
	case vtString: return Str.AsFloat();
	}
	return 0;
}

Property::operator const String() const
{
	switch(Type)
	{
	case vtInt: return String(Int);
	case vtFloat: return String(Float);
	case vtString: return Str;
	}
	return L"";
}

bool Property::operator==(const Property& v) const
{
	switch(Type)
	{
	case vtInt: return operator==((int)v);
	case vtFloat: return operator==((float)v);
	case vtString: return operator==((const String)v);
	}
	return false;
}

void PropertyStore::SaveToList(StringList& list, StringList& names)
{
	ValueList temp;
	for (int i=0; i<Count(); i++)
		temp.Set(names[i], (*this)[names.Values(i)]);
	temp.SaveToList(list);
}

void PropertyStore::LoadFromList(StringList& list, StringList& names)
{
	ValueList temp;
	temp.LoadFromList(list);
	for (int i=0; i<temp.Count(); i++)
	{
		int j=names.Find(temp.Name(i));
		if (j!=-1)
			(*this)[names.Values(j)]=temp.Value(i);
	}
}
