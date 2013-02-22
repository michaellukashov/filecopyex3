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

#ifndef	__CLASSES_H__
#define	__CLASSES_H__

#pragma once

#include "framework/fileutils.h"
#include "framework/valuelist.h"

class FileName 
{
public:
	enum Direction {
		levelPlus, 
		levelMinus, 
		levelSame,
		levelStar, 
	};

	FileName(const Direction _d, const String& _name): d(_d), Name(_name) {};
	~FileName() {};

	Direction getDirection() const {return d;};
	String getName() const {return Name;};

private:
	Direction d;
	String Name;
};

class FileNameStore
{
public:
	FileNameStore() { }
	~FileNameStore() { }

	size_t AddRel(FileName::Direction _d, const String& _name) { items.push_back(FileName(_d, _name)); return items.size()-1; };
	size_t Count() const { return items.size(); };
	const String GetNameByNum(size_t n) const { return items[n].getName(); };
	const FileName& operator[](size_t n) const { return items[n]; };
	FileName& operator[](size_t n) { return items[n]; };

private:
		std::vector<FileName> items;
};

// ===== FileNameStoreEnum =====
class FileNameStoreEnum
{
private:
	FileNameStore* store;
	size_t cur;
	String curPath, buffer;

public:
	FileNameStoreEnum(FileNameStore *_store)
	{
		store = _store;
		ToFirst();
	}
	~FileNameStoreEnum() { ; }

	size_t Count() const { return store->Count(); }
	void ToFirst();
	void Skip();
	String GetNext();
	String GetByNum(size_t num);

};

// ===== DescList =====
class DescList
{
public:
	DescList();
	int LoadFromFile(const String& fn);
	int LoadFromString(wchar_t *ptr);
	int LoadFromList(StringList& list);
	int SaveToFile(const String& fn);
	void Merge(DescList&);
	String Name(int i) { return Names[i]; }
	String Value(int i) { return Values[Names.Values(i)]; }
	String Value(const String& name);
	int& Flags(int i) { return Values.Values(Names.Values(i)); }
	int Count() { return Names.Count(); }
	void SetMergeFlag(const String&, int flag);
	void SetSaveFlag(const String&, int flag);
	void SetAllMergeFlags(int flag);
	void SetAllSaveFlags(int flag);
	int Add(const String& name, const String& val);
	void Rename(int i, const String& dst, int changeName=0);
	void Rename(const String& src, const String& dst, int changeName=0);

private:
	StringList Names, Values;
};

#endif//__CLASSES_H__
