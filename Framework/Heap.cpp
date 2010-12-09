#include <stdhdr.h>
#include "heap.h"

static void inline Abort()
{
  TerminateProcess(GetCurrentProcess(), -1);
}

Heap *heap;
Handle InvHandle = {-1};

Heap::Heap(void)
{
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  PageSize=si.dwPageSize;

  IndexLimit=4*1024*1024;
  PoolLimit=64*1024*1024;
  IndexAllocBlock=16*1024;
  PoolAllocBlock=128*1024;

  Index=(IndexItem*)VirtualAlloc(NULL, IndexLimit, MEM_RESERVE, PAGE_NOACCESS);
  Pools[0]=(unsigned char*)VirtualAlloc(NULL, PoolLimit, MEM_RESERVE, PAGE_NOACCESS);
  Pools[1]=(unsigned char*)VirtualAlloc(NULL, PoolLimit, MEM_RESERVE, PAGE_NOACCESS);
  CurPool=0;
  Pool=Pools[0];
  if (!Index || !Pools[0] || !Pools[1])
  {
    FWError(_T("Virtual memory reserve error"));
    Abort();
  }

  IndexCount=IndexFreeCount=IndexAlloc=0;
  PoolSize=PoolTrash=PoolAlloc=0;
  TrashLimit=64*1024;
  Multithreaded=0;
  GLock=0;
}

Heap::~Heap(void)
{
  VirtualFree(Index, IndexLimit, MEM_DECOMMIT);
  VirtualFree(Index, 0, MEM_RELEASE);
  VirtualFree(Pools[0], PoolLimit, MEM_DECOMMIT);
  VirtualFree(Pools[0], 0, MEM_RELEASE);
  VirtualFree(Pools[1], PoolLimit, MEM_DECOMMIT);
  VirtualFree(Pools[1], 0, MEM_RELEASE);
}

inline int Heap::CommitPool(int num)
{
  if (!VirtualAlloc(Pools[num], PoolAlloc,
    MEM_COMMIT, PAGE_READWRITE))
  {
    FWError(_T("Virtual memory commit error"));
    return 0;
  }
  return 1;
}

inline void Heap::DecommitPool(int num)
{
  VirtualFree(Pools[num], PoolLimit, MEM_DECOMMIT);
}

inline int Heap::ReallocIndex(int size)
{
  IndexCount=size;
  size*=sizeof(IndexItem);
  if (IndexAlloc<size || IndexAlloc>=size+IndexAllocBlock)
    return InternalReallocIndex(size);
  else return 1;
}

inline int Heap::ReallocPool(int size)
{
  PoolSize=size;
  if (PoolAlloc<size || PoolAlloc>=size+PoolAllocBlock)
    return InternalReallocPool(size);
  else return 1;
}

int Heap::InternalReallocIndex(int size)
{
  if (IndexAlloc<size)
  {
    int OldAlloc=IndexAlloc;
    while (IndexAlloc<size) IndexAlloc+=IndexAllocBlock;
    if (IndexAlloc>IndexLimit)
    {
      FWError(_T("Heap limit exceeded"));
      return 0;
    }
    if (!VirtualAlloc((unsigned char*)Index+OldAlloc, IndexAlloc-OldAlloc,
      MEM_COMMIT, PAGE_READWRITE))
    {
      FWError(_T("Virtual memory commit error"));
      return 0;
    }
  }
  else if (IndexAlloc>=size+IndexAllocBlock)
  {
    int OldAlloc=IndexAlloc;
    while (IndexAlloc>=size+IndexAllocBlock) IndexAlloc-=IndexAllocBlock;
    VirtualFree((unsigned char*)Index+IndexAlloc, OldAlloc-IndexAlloc, MEM_DECOMMIT);
  }
  return 1;
}

int Heap::InternalReallocPool(int size)
{
  if (PoolAlloc<size)
  {
    int OldAlloc=PoolAlloc;
    while (PoolAlloc<size) PoolAlloc+=PoolAllocBlock;
    if (PoolAlloc>PoolLimit)
    {
      FWError(_T("Heap limit exceeded"));
      return 0;
    }
    if (!VirtualAlloc((unsigned char*)Pool+OldAlloc, PoolAlloc-OldAlloc,
      MEM_COMMIT, PAGE_READWRITE))
    {
      FWError(_T("Virtual memory commit error"));
      return 0;
    }
  }
  else if (PoolAlloc>=size+PoolAllocBlock)
  {
    int OldAlloc=PoolAlloc;
    while (PoolAlloc>=size+PoolAllocBlock) PoolAlloc-=PoolAllocBlock;
    VirtualFree((unsigned char*)Pool+PoolAlloc, OldAlloc-PoolAlloc, MEM_DECOMMIT);
  }
  return 1;
}

inline Handle Heap::AllocHandle()
{
  Handle res;
  if (IndexFreeCount > 0)
    res.h=Index[--IndexFreeCount].FreeList;
  else
  {
    res.h=IndexCount;
    if (!ReallocIndex(IndexCount+1)) return InvHandle;
  }
  Index[res.h].Free=0;
  Index[res.h].Lock=0;
  return res;
}

inline void Heap::UnallocHandle(Handle hnd)
{
  Index[hnd.h].Free=1;
  GLock-=Index[hnd.h].Lock;
  Index[IndexFreeCount++].FreeList=hnd.h;
}

Handle Heap::Alloc(int size)
{
  MTLock();
  Handle res=AllocHandle();
  if (res!=InvHandle)
  {
    Index[res.h].Ref=PoolSize;
    Index[res.h].Size=size;
    ReallocPool(PoolSize+size);
  }
  MTUnlock();
  return res;
}

void Heap::Free(Handle hnd)
{
  MTLock();
  if (hnd!=InvHandle && !Index[hnd.h].Free)
  {
    PoolTrash+=Index[hnd.h].Size;
    UnallocHandle(hnd);
    Compact();
  }
  else
  {
    if (hnd==InvHandle)
      FWError(_T("Attempt to free an invalid handle"));
    else
      FWError(_T("Attempt to free already freed handle"));
  }
  MTUnlock();
}

void Heap::Realloc(Handle hnd, int size)
{
  MTLock();
  if (hnd!=InvHandle && !Index[hnd.h].Free)
  {
    if (size<=Index[hnd.h].Size)
    {
      Index[hnd.h].Size=size;
      PoolTrash+=Index[hnd.h].Size-size;
    }
    else
    {
      int OldRef=Index[hnd.h].Ref, 
          OldSize=Index[hnd.h].Size;
      Index[hnd.h].Ref=PoolSize;
      Index[hnd.h].Size=size;
      ReallocPool(PoolSize+size);
      memcpy(Pool+Index[hnd.h].Ref, Pool+OldRef, OldSize);
      PoolTrash+=OldSize;
    }
    Compact();
  }
  else
  {
    if (hnd==InvHandle)
      FWError(_T("Attempt to realloc an invalid handle"));
    else
      FWError(_T("Attempt to realloc a freed handle"));
  }
  MTUnlock();
}

inline void Heap::Compact()
{
  MTLock();
  if (!GLock && PoolTrash>TrashLimit) 
    ForceCompact();
  MTUnlock();
}

void Heap::ForceCompact()
{
  MTLock();
  int nsz=0;
  for (int i=0; i<IndexCount; i++)
    if (!Index[i].Free)
      nsz+=Index[i].Size;

  CurPool=1-CurPool;
  Pool=Pools[CurPool];
  PoolAlloc=0;
  ReallocPool(nsz);

  unsigned char* Ptr=Pool, *SPtr=Pools[1-CurPool];
  int Ref=0;
  for (i=0; i<IndexCount; i++)
    if (!Index[i].Free)
    {
      memmove(Ptr, SPtr+Index[i].Ref, Index[i].Size);
      Index[i].Ref=Ref;
      Ptr+=Index[i].Size;
      Ref+=Index[i].Size;
    }

  DecommitPool(1-CurPool);
  PoolTrash=0;
  MTUnlock();
}

void Heap::SetMultithreaded(int v)
{
  Multithreaded=v;
  if (v) InitializeCriticalSection(&CSec);
  else DeleteCriticalSection(&CSec);
}

static HANDLE hHeap=NULL;

static void GMakeHeap()
{
  if (!hHeap)
  {
    hHeap=HeapCreate(0, 65536, 0);
    if (!hHeap)
    {
      FWError(_T("Global heap alloc error"));
      Abort();
    }
  }
}

void* GAlloc(int size)
{
  GMakeHeap();
  return HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
}

void* GRealloc(void* ptr, int size)
{
  GMakeHeap();
  if (ptr)
    return HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, ptr, size);
  else
    return HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
}

void GFree(void* ptr)
{
  GMakeHeap();
  if (ptr) HeapFree(hHeap, 0, ptr);
}
