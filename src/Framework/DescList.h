#ifndef __DESCLIST_H__
#define __DESCLIST_H__

#include "StringList.h"

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

#endif
