#pragma once
#include "store.h"
#include "fileutils.h"
#include "valuelist.h"

class FileNameStoreEnum;

class FileNameStore : protected StringStore
{
public:
  FileNameStore() { ; }
  ~FileNameStore() { ; }

  int Add(TCHAR* ptr) { return StringStore::Add(ptr); }
  int AddRel(TCHAR pc, TCHAR* ptr);
  int Count() { return StringStore::Count(); }
  TCHAR* GetNameByNum(int num);

  friend class FileNameStoreEnum;
};

class FileNameStoreEnum
{
private:
  FileNameStore* Store;
  int Cur;
  TCHAR Buffer[MAX_FILENAME], CurPath[MAX_FILENAME];

public:
  FileNameStoreEnum(FileNameStore& store)
  {
    Store=&store;
    ToFirst();
  }
  ~FileNameStoreEnum() { ; }

  int Count() { return Store->Count(); }
  void ToFirst();
  void Skip();
  TCHAR* GetNext();
  TCHAR* GetByNum(int num);

};

class DescList
{
public:
  DescList();
  int LoadFromFile(const String& fn);
  int LoadFromString(TCHAR *ptr);
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

