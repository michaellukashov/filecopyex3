#pragma once
#include "heap.h"

template <class ItemType> class Array
{
public:
  Array(void);
  ~Array(void);
  void SetBlock(int);

  typedef int (*CompareProc)(const ItemType&, const ItemType&, const void*);
  void SetSorted(int, CompareProc);
  int IsSorted() { return Sorted; }

  void Clear();          
  void Resize(int);
  int Count();
  ItemType& operator[](int);

  int Add(const ItemType&);
  void Delete(int, int=1);
  void Insert(int, const ItemType&);
  void Exchange(int, int);

  int AddGroup(const ItemType*, int);
  void SetGroup(int, const ItemType*, int);

  int Find(const ItemType&, int=0, const void* =NULL);
  int GFind(const ItemType&, int=0, const void* =NULL);

  const ItemType* Storage();

private:
  ItemType *stg;
  int count, alloc, block;

  void RecursiveSort(int, int);
  int InternalFind(const ItemType& key, int, int,  const void*);
  void InternalExchange(int, int);
  void InternalInsert(int, const ItemType&);
  int AddSorted(const ItemType&);
  int Sorted;

  CompareProc Compare;
  friend static int __Compare(const ItemType&, const void*, const void*);
};

template <class ItemType>
Array<ItemType>::Array(void)
{
  count=0;
  alloc=0;
  stg=NULL;
  block=4096/sizeof(ItemType);
  Sorted=0;
  Compare=NULL;
}

template <class ItemType>
Array<ItemType>::~Array(void)
{
  if (stg) GFree(stg);
}

template <class ItemType>
void Array<ItemType>::SetBlock(int blk)
{
  block=blk;
}

template <class ItemType>
void Array<ItemType>::SetSorted(int sorted, CompareProc compare)
{
  Sorted=sorted;
  Compare=compare;
  if (Sorted)
    RecursiveSort(0, count-1);
}

template <class ItemType>
void Array<ItemType>::Clear()
{
  if (stg) GFree(stg);
  count=0;
  alloc=0;
  stg=NULL;
}

template <class ItemType>
void Array<ItemType>::Resize(int n)
{
  if (n>alloc)
  {
    while (n>alloc) alloc+=block;
    stg=(ItemType*)GRealloc(stg, alloc*sizeof(ItemType));
  }
  else if (n<=alloc-block)
  {
    while (n<=alloc-block) alloc-=block;
    stg=(ItemType*)GRealloc(stg, alloc*sizeof(ItemType));
  }
  count=n;
}

template <class ItemType>
 ItemType& Array<ItemType>::operator [](int n)
{
  return stg[n];
}

template <class ItemType>
 int Array<ItemType>::Count()
{
  return count;
}

template <class ItemType>
int Array<ItemType>::Add(const ItemType &v)
{
  if (!Sorted)
    return AddGroup(&v, 1);
  else
    return AddSorted(v);
}

template <class ItemType>
int Array<ItemType>::AddGroup(const ItemType *p, int n)
{
  if (!Sorted)
  {
    Resize(count+n);
    for (int i=0; i<n; i++)
      stg[count-n+i]=p[i];
    return count-n;
  }
  else
  {
    for (int i=0; i<n; i++)
      AddSorted(p[i]);
    return -1;
  }
}

template <class ItemType>
void Array<ItemType>::SetGroup(int s, const ItemType *p, int n)
{
  if (!Sorted)
  {
    for (int i=0; i<n; i++) stg[s+i]=p[i];
  }
  else
  {
    Delete(s, n);
    for (int i=0; i<n; i++) AddSorted(p[i]);
  }
}

template <class ItemType>
void Array<ItemType>::Delete(int n, int c)
{
  for (int i=n+c; i<count; i++)
    stg[i-c]=stg[i];
  Resize(count-c);
}

template <class ItemType>
void Array<ItemType>::InternalInsert(int n, const ItemType& v)
{
  Resize(count+1);
  for (int i=count-2; i>=n; i--)
    stg[i+1]=stg[i];
  stg[n]=v;
}

template <class ItemType>
void Array<ItemType>::Insert(int n, const ItemType& v)
{
  if (!Sorted)
    InternalInsert(n, v);
  else
    AddSorted(v);
}

template <class ItemType>
void Array<ItemType>::Exchange(int i, int j)
{
  if (!Sorted)
    InternalExchange(i, j);
}

template <class ItemType>
void Array<ItemType>::InternalExchange(int i, int j)
{
  ItemType t=(*this)[i]; 
  (*this)[i]=(*this)[j]; 
  (*this)[j]=t;
}

template <class ItemType>
 const ItemType* Array<ItemType>::Storage()
{
  return stg;
}

template <class ItemType>
int Array<ItemType>::AddSorted(const ItemType& v)
{
  int n=GFind(v);
  if (n==-1) n=Count();
  InternalInsert(n, v);
  return n;
}

#define FIND_EXACT 0
#define FIND_GREATER 2

template <class ItemType>
int Array<ItemType>::InternalFind(const ItemType& key, int start, int mode, 
                                  const void* param)
{
  if (Sorted)
  {
    int a=start, b=Count()-1;
    while (a<b)
    {
      int c=(a+b)/2;
      ItemType& cs=(*this)[c];
      if (Compare(cs, key, param)>=0) b=c;
      else a=c+1;
    } 
    if (a<0 || a>=Count() || mode!=FIND_GREATER 
      && Compare((*this)[a], key, param) || mode==FIND_GREATER
      && Compare((*this)[a], key, param)<0) 
      return -1;
    else
      return a;
  }
  else
  {
    if (mode==FIND_EXACT)
    {
      for (int i=start; i<Count(); i++)
        if (!Compare((*this)[i], key, param)) return i;
    }
    else if (mode==FIND_GREATER)
    {
      for (int i=start; i<Count(); i++)
        if (Compare((*this)[i], key, param)>=0) return i;
    }
    return -1;
  }
}

template <class ItemType>
void Array<ItemType>::RecursiveSort(int a, int b)
{
  while (a<b)
  {
    ItemType& op=(*this)[(a+b)/2];
    int i=a, j=b;
    while (i<j)
    {
      while (i<=b && Compare((*this)[i], op, NULL)<0) i++;
      while (j>=a && Compare((*this)[j], op, NULL)>0) j--;
      if (i<=j) InternalExchange(i++, j--);
    }
    if (j-a>b-i)
    {
      RecursiveSort(i, b);
      b=j;
    }
    else
    {
      RecursiveSort(a, j);
      a=i;
    }
  }
}

template <class ItemType>
int Array<ItemType>::Find(const ItemType& v, int start, const void* Param)
{
  return InternalFind(v, start, FIND_EXACT, Param);
}

template <class ItemType>
int Array<ItemType>::GFind(const ItemType& v, int start, const void* Param)
{
  return InternalFind(v, start, FIND_GREATER, Param);
}

