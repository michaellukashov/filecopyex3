#ifndef __DESCLIST_H__
#define __DESCLIST_H__

#include <map>

#include "StringVector.h"

class DescList
{
public:
	DescList();
	int LoadFromFile(const String& fn);
	void Merge(DescList&);
	int SaveToFile(const String& fn);
	void SetAllMergeFlags(int v);
	void SetAllSaveFlags(int v);
	void SetMergeFlag(const String&, int v);
	void SetSaveFlag(const String&, int v);
	void Rename(const String& src, const String& dst, int changeName=0);

private:
	void setFlag(const String&, int flag, int v);
	void setAllFlags(int flag, int v);

	class Data {
	public:
		String desc;
		int flags;

		Data() : flags(0) {};
		Data(String _desc, int _flags=0): desc(_desc), flags(_flags) {};
	};
	typedef std::map<String, Data> DescListMap;
	DescListMap names;

	int LoadFromString(wchar_t *ptr);
	int LoadFromList(StringVector& list);
};

#endif
