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

#include "../framework/store.h"
#include "../framework/fileutils.h"
#include "../framework/valuelist.h"

class FileNameStoreEnum;

class FileNameStore : protected StringStore
{
public:
	FileNameStore() { ; }
	~FileNameStore() { ; }

	int Add(const wchar_t* ptr) { return StringStore::Add(ptr); }
	int AddRel(wchar_t pc, const wchar_t* ptr);
	int Count() const { return StringStore::Count(); }
	const wchar_t* GetNameByNum(int n) const { return (*this)[n]+1; }

	friend class FileNameStoreEnum;
};

class FileNameStoreEnum
{
private:
	FileNameStore* Store;
	int Cur;
	wchar_t Buffer[MAX_FILENAME], CurPath[MAX_FILENAME];

public:
	FileNameStoreEnum(FileNameStore& store)
	{
		Store=&store;
		ToFirst();
	}
	~FileNameStoreEnum() { ; }

	int Count() const { return Store->Count(); }
	void ToFirst();
	void Skip();
	const wchar_t* GetNext();
	const wchar_t* GetByNum(int num);

};

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
