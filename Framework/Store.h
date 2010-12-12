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

#ifndef	__STORE_H__
#define	__STORE_H__

#pragma once

#include <vector>
#include "LowLevelStr.h"
#include "ObjString.h"

class Store  
{
public:
	Store();
	virtual ~Store();

	int Add(int size);
	void* operator[](int n) const { return items[n].ptr; }
	int Size(int n) const { return (int)items[n].size; }
	int Count() const { return (int)items.size(); }

protected:
	struct Item
	{
		void* ptr;
		size_t size;
	};
	std::vector<Item> items;
};

class StringStore : protected Store
{
public:
	StringStore() { ; }
	~StringStore() { ; }

	int Add(const wchar_t* ptr)
	{
		size_t l = wcslen(ptr)+1;
		int res = Store::Add((int)l*sizeof(wchar_t));
		wcscpy_s((wchar_t*)operator[](res), l, ptr);
		return res;
	}
	int Add(const String& ptr) { return Add(ptr.ptr()); }
	const wchar_t* operator[](int n) const { return (const wchar_t*)Store::operator[](n); }
	int Len(int n) const { return Size(n)/sizeof(wchar_t)-1; }
	int Count() const { return Store::Count(); }
};

template <class ItemType>
class ArrayStore : protected Store
{
public:
	ArrayStore() { ; }
	~ArrayStore() { ; }

	int Add(const ItemType& ptr)
	{ 
		int res = Store::Add(sizeof(ItemType)); 
		operator[](res) = ptr;
		return res;
	}
	ItemType& operator[](int n) { return *(ItemType*)Store::operator[](n); }
	int Count() { return Store::Count(); }
};

#endif//__STORE_H__
