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
#include "ObjString.h"

class StringStore 
{
public:
	StringStore();
	~StringStore();

	size_t Add(const wchar_t* ptr);
	size_t Add(const String& ptr) { return Add(ptr.ptr()); }
	const wchar_t* operator[](size_t n) const { return items[n].ptr; }
	size_t Len(size_t n) const { return items[n].size/sizeof(wchar_t)-1; }
	size_t Count() const { return items.size(); }

protected:
	struct Item
	{
		wchar_t *ptr;
		size_t size;
	};
	std::vector<Item> items;
};

#endif//__STORE_H__
