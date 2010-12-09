#include <stdhdr.h>
#include "store.h"
#include "fwcommon.h"

static void inline Abort()
{
  TerminateProcess(GetCurrentProcess(), -1);
}

Store::Store()
{
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  PageSize=si.dwPageSize;

  IndexLimit=4*1024*1024;
  PoolLimit=64*1024*1024;
  IndexAllocBlock=16*1024;
  PoolAllocBlock=128*1024;

  Index=(IndexItem*)VirtualAlloc(NULL, IndexLimit, MEM_RESERVE, PAGE_NOACCESS);
  Pool=(unsigned char*)VirtualAlloc(NULL, PoolLimit, MEM_RESERVE, PAGE_NOACCESS);
  if (!Index || !Pool)
  {
    FWError(_T("Virtual memory reserve error"));
    Abort();
  }

  IndexCount=IndexAlloc=0;
  PoolSize=PoolAlloc=0;
}

Store::~Store()
{
  VirtualFree(Index, IndexLimit, MEM_DECOMMIT);
  VirtualFree(Index, 0, MEM_RELEASE);
  VirtualFree(Pool, PoolLimit, MEM_DECOMMIT);
  VirtualFree(Pool, 0, MEM_RELEASE);
}

inline int Store::ReallocIndex(int size)
{
  IndexCount=size;
  size*=sizeof(IndexItem);
  if (IndexAlloc<size)
    return InternalReallocIndex(size);
  else return 1;
}

inline int Store::ReallocPool(int size)
{
  PoolSize=size;
  if (PoolAlloc<size)
    return InternalReallocPool(size);
  else return 1;
}

int Store::InternalReallocIndex(int size)
{
  int OldAlloc=IndexAlloc;
  while (IndexAlloc<size) IndexAlloc+=IndexAllocBlock;
  if (IndexAlloc>IndexLimit)
  {
    FWError(_T("Store index size limit exceeded"));
    return 0;
  }
  if (!VirtualAlloc((unsigned char*)Index+OldAlloc, IndexAlloc-OldAlloc,
    MEM_COMMIT, PAGE_READWRITE))
  {
    FWError(_T("Virtual memory commit error"));
    return 0;
  }
  return 1;
}

int Store::InternalReallocPool(int size)
{
  int OldAlloc=PoolAlloc;
  while (PoolAlloc<size) PoolAlloc+=PoolAllocBlock;
  if (PoolAlloc>PoolLimit)
  {
    FWError(_T("Store pool size limit exceeded"));
    return 0;
  }
  if (!VirtualAlloc((unsigned char*)Pool+OldAlloc, PoolAlloc-OldAlloc,
    MEM_COMMIT, PAGE_READWRITE))
  {
    FWError(_T("Virtual memory commit error"));
    return 0;
  }
  return 1;
}

int Store::Add(int size)
{
  ReallocIndex(IndexCount+1);
  int idx=IndexCount-1;
  Index[idx].Ref=PoolSize;
  Index[idx].Size=size;
  ReallocPool(PoolSize+size);
  return idx;
}


