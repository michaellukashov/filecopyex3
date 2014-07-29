/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010
Idea & core: Max Antipin
Coding: Serge Cheperis aka craZZy
Bugfixes: slst, CDK, Ivanych, Alter, Axxie and Nsky
Special thanks to Vitaliy Tsubin
Far 2 (32 & 64 bit) full unicode version by djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "FarProgress.h"

int64_t GetTime();

class CopyProgress : public FarProgress
{
public:
  CopyProgress(void);
  virtual ~CopyProgress(void);
  void Start(int move);
  void Stop();
  void ShowReadName(const String &);
  void ShowWriteName(const String &);

  void ShowProgress(int64_t read, int64_t write, int64_t total,
                    int64_t readTime, int64_t writeTime,
                    int64_t readN, int64_t writeN,
                    int64_t totalN, int parallel,
                    int64_t FirstWrite, int64_t StartTime, size_t BufferSize);
private:
  void RedrawWindowIfNeeded();
  void RedrawWindow();
  void DrawProgress(const String &, int, int64_t, int64_t, int64_t, int64_t, int64_t);

  void DrawName(const String &, int);

  void DrawTime(int64_t ReadBytes, int64_t WriteBytes, int64_t TotalBytes,
                int64_t ReadTime, int64_t WriteTime,
                int64_t ReadN, int64_t WriteN, int64_t TotalN,
                int ParallelMode, int64_t FirstWriteTime,
                int64_t StartTime, size_t BufferSize);

  // bug #22 fixed by axxie
  int64_t lastupdate, lastupdate_read, lastupdate_write, interval, clastupdate, cinterval;

  int X1, Y1, X2, Y2, Move;
};
