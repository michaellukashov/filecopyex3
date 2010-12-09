#pragma once
#include "array.h"
#include "objstring.h"

enum TextFormat { tfANSI, tfUnicode, tfUnicodeBE };

#define slSorted 1
#define slIgnoreCase 2

class StringList
{
public:
	StringList(void);
	virtual ~StringList(void);
//	void SetBlock(int);
//	void SetOptions(int);

	int Count();
	const String operator[](int);
	int& Values(int);
	void*& PtrValues(int);
	void Set(int, const String&);

	int Add(const String&, int=0);
	void Insert(int, const String&, int=0);
	int Add(const String&, void*);
	void Insert(int, const String&, void*);
	void Delete(int);
	void Exchange(int, int);
	void Clear();

	int Find(const String&, int=0);
// 	int PFind(const String&, int=0);
// 	int GFind(const String&, int=0);

	int LoadFrom(FILE*);
	int SaveTo(FILE*, TextFormat=tfANSI);
	int Load(const String&);
	int Save(const String&, TextFormat=tfANSI);
	void LoadFromString(const String&, wchar_t);
	void LoadFromString(const wchar_t*, wchar_t);

	void AddList(StringList&);

	struct ListItem
	{
		String str;
		union
		{
			int Data;
			void* PtrData;
		};
	};

private:
	Array<ListItem> items;
};
