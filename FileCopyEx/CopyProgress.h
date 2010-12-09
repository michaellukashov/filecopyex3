#pragma once
#include "far/farprogress.h"

__int64 GetTime();

class CopyProgress : public  FarProgress
{
public:
  CopyProgress(void);
  virtual ~CopyProgress(void);
  void Start(int move);
  void Stop();
  void ShowReadName(const String&);
  void ShowWriteName(const String&);

  void ShowProgress(__int64 read, __int64 write, __int64 total,
                    __int64 readTime, __int64 writeTime,
                    __int64 readN, __int64 writeN,
                    __int64 totalN, int parallel, 
                    __int64 FirstWrite, __int64 StartTime, int BufferSize);
private:
  void DrawProgress(const String&, int, __int64, __int64, __int64, __int64, __int64); 

  void DrawName(const String&, int);

  void DrawTime(__int64 ReadBytes, __int64 WriteBytes, __int64 TotalBytes,
                __int64 ReadTime, __int64 WriteTime,
                __int64 ReadN, __int64 WriteN, __int64 TotalN,
                int ParallelMode, __int64 FirstWriteTime, 
                __int64 StartTime, int BufferSize);

  // bug #22 fixed by axxie
  __int64 lastupdate, lastupdate_read, lastupdate_write, interval, clastupdate, cinterval;

  int X1, Y1, X2, Y2, Move;
};
