#pragma once

#include "FileNameStore.h"
#include "ObjString.h"

class FileNameStoreEnum
{
private:
  FileNameStore * store;
  size_t cur;
  String curPath, buffer;

public:
  FileNameStoreEnum(FileNameStore * _store)
  {
    store = _store;
    ToFirst();
  }
  ~FileNameStoreEnum() { ; }

  size_t Count() const { return store->Count(); }
  void ToFirst();
  void Skip();
  String GetNext();
  String GetByNum(size_t num);

};


