#pragma once
#include "fwcommon.h"

// Warnings:
// -- heap limit is 1048576 items and 64MB of content
// -- do not use same heap in more than one thread

class Heap;
class String;

struct Handle
{
  int h;
  bool operator==(Handle v) { return h==v.h; }
  bool operator!=(Handle v) { return h!=v.h; }
};

extern Handle InvHandle;
extern Heap *heap;

void* GAlloc(int size);
void* GRealloc(void* ptr, int size);
void GFree(void* ptr);

class Heap
{
public:
  Heap(void);
  virtual ~Heap(void);

  Handle Alloc(int size);
  void Free(Handle hnd);
  void Realloc(Handle hnd, int size);

  void SetTrashLimit(int limit) { TrashLimit=limit; }
  void SetMultithreaded(int v);

  int CheckHnd(Handle hnd)
  {
    if (hnd==InvHandle)
    {
      FWError(_T("Attempt to use an invalid handle"));
      return 0;
    }
    else if (Index[hnd.h].Free)
    {
      FWError(_T("Attempt to use a freed handle"));
      return 0;
    }
    return 1;
  }

  void MTLock()
  {
    if (Multithreaded)
      EnterCriticalSection(&CSec);
  }

  void MTUnlock()
  {
    if (Multithreaded)
      LeaveCriticalSection(&CSec);
  }

  void* Lock(Handle hnd)
  {
    #ifdef _DEBUG
      if (!CheckHnd(hnd)) return NULL;
    #endif
    MTLock();
    Index[hnd.h].Lock++;
    GLock++;
    void* res=Pool+Index[hnd.h].Ref;
    MTUnlock();
    return res;
  }
  
  void Unlock(Handle hnd)
  {
    #ifdef _DEBUG
      if (!CheckHnd(hnd)) return;
    #endif
    MTLock();
    Index[hnd.h].Lock--;
    int c=Index[hnd.h].Lock;
    GLock--;
    MTUnlock();
    #ifdef _DEBUG
      if (c<0)
      {
        FWError(_T("Mismatched Lock()/Unlock()"));
        return;
      }
    #endif
  }

  int GetSize(Handle hnd)
  {
    #ifdef _DEBUG
      if (!CheckHnd(hnd)) return 0;
    #endif
    MTLock();
    int res=Index[hnd.h].Size;
    MTUnlock();
    return res;
  }

  Handle AllocString(const TCHAR* str)
  {
    Handle res=Alloc(((int)(str?_tcslen(str):0)+1)*sizeof(TCHAR));
    _tcscpy(LockString(res), str?str:_T(""));
    Unlock(res);
    return res;
  }

  void ReallocString(Handle hnd, const TCHAR* str)
  {
    Realloc(hnd, ((int)(str?_tcslen(str):0)+1)*sizeof(TCHAR));
    _tcscpy(LockString(hnd), str?str:_T(""));
    Unlock(hnd);
  }

  TCHAR* LockString(Handle hnd)
  {
    return (TCHAR*)Lock(hnd); 
  }

  Handle AllocString(const String& str);
  void ReallocString(Handle hnd, const String& str);

#ifdef _DEBUG
  const void* DebugPtr(Handle hnd) { return Pool+Index[hnd.h].Ref; }
#endif

private:
  struct IndexItem {
    int Free, Ref, FreeList, Size, Lock;
  } *Index;
  int IndexCount, IndexFreeCount;
  int IndexLimit, IndexAlloc, IndexAllocBlock, PageSize;
  unsigned char *Pool, *Pools[2];
  int PoolLimit, PoolAlloc, PoolAllocBlock, CurPool;
  int PoolSize, PoolTrash, TrashLimit;
  int GLock;
  
  int ReallocIndex(int size);
  int ReallocPool(int size);
  int InternalReallocIndex(int size);
  int InternalReallocPool(int size);
  int CommitPool(int num);
  void DecommitPool(int num);

  Handle AllocHandle();
  void UnallocHandle(Handle hnd);
  void Compact();
  void ForceCompact();

  int Multithreaded;
  CRITICAL_SECTION CSec;

  friend class String;
};

inline void InitHeap()
{
  if (!heap) heap=new Heap();
}
