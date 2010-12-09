#pragma once
#include "objstring.h"

class Store  
{
public:
  Store();
  virtual ~Store();

  int Add(int size);
  void* operator[](int n) { return Pool+Index[n].Ref; }
  int Size(int n) { return Index[n].Size; }
  int Count() { return IndexCount; }

  struct IndexItem 
  {
    int Ref, Size;
  };

protected:
  unsigned char *Pool;
  IndexItem *Index;
  int IndexCount, IndexLimit, PoolSize, PoolLimit,
    IndexAlloc, IndexAllocBlock, PoolAlloc, PoolAllocBlock, PageSize;

  int ReallocIndex(int size);
  int ReallocPool(int size);
  int InternalReallocIndex(int size);
  int InternalReallocPool(int size);
};

class StringStore : protected Store
{
public:
  StringStore() { ; }
  ~StringStore() { ; }

  int Add(const TCHAR* ptr)
  {
    int res=Store::Add((int)(_tcslen(ptr)+1)*sizeof(TCHAR));
    _tcscpy(operator[](res), ptr);
    return res;
  }
  int Add(const String& ptr) { return Add(ptr.ptr()); }
  TCHAR* operator[](int n) { return (TCHAR*)Store::operator[](n); }
  int Len(int n) { return Index[n].Size/sizeof(TCHAR)-1; }
  int Count() { return IndexCount; }
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
    { return IndexCount; }
};
