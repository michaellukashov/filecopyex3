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
	void* operator[](int n) { return items[n].ptr; }
	int Size(int n) { return (int)items[n].size; }
	int Count() { return (int)items.size(); }

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
		size_t l = _tcslen(ptr)+1;
		int res=Store::Add((int)l*sizeof(wchar_t));
		_tcscpy_s(operator[](res), l, ptr);
		return res;
	}
	int Add(const String& ptr) { return Add(ptr.ptr()); }
	wchar_t* operator[](int n) { return (wchar_t*)Store::operator[](n); }
	int Len(int n) { return Size(n)/sizeof(wchar_t)-1; }
	int Count() { return Store::Count(); }
};

template <class ItemType>
class ArrayStore : protected Store
{
public:
	ArrayStore() { ; }
	~ArrayStore() { ; }

	int Add(const ItemType& ptr)
	{ 
		int res=Store::Add(sizeof(ItemType)); 
		operator[](res)=ptr;
		return res;
	}
	ItemType& operator[](int n) 
	{ return *(ItemType*)Store::operator[](n); }
	int Count() 
	{ return Store::Count(); }
};
