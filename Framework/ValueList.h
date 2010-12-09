#pragma once
#include "stringlist.h"

class ValueList
{
public:
	ValueList();
	virtual ~ValueList();

	const String Name(int);
	const String Value(int);
	int Count();
	void Clear();

	void Set(const String&, const String&);
	void Set(const String&);

	const String operator[](const String&); 

	void LoadFromList(StringList&);
	void SaveToList(StringList&);

	struct ListItem
	{
		String name, value;
	};

private:
	int Find(const String& k);
	Array<ListItem> items;
};

class LocaleList : public ValueList
{
public:
	void Load(const String& fn);
};
