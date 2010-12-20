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

#ifndef	__ARRAY_H__
#define	__ARRAY_H__

#pragma once

#include <vector>

template <class ItemType> class Array
{
public:
	void Clear() { arr.clear(); }
	void Resize(int n) { 	arr.resize(n); }
	int Count() const { return (int)arr.size(); }
	ItemType& operator[](int n) { return arr[n]; }
	const ItemType& operator[](int n) const { return arr[n]; }

	int Add(const ItemType& v) { arr.push_back(v); return (int)arr.size() - 1; }
	void Delete(int n, int c = 1) { arr.erase(arr.begin() + n, arr.begin() + n + c); }
	void Insert(int n, const ItemType& v) { arr.insert(arr.begin() + n, v); }
	void Exchange(int i, int j)
	{
		ItemType item = arr[i];
		arr[i] = arr[j];
		arr[j] = item;
	}

	const ItemType* Storage() const { return arr.empty() ? NULL : &arr.front(); }

private:
	std::vector<ItemType> arr;
};

#endif//__ARRAY_H__
