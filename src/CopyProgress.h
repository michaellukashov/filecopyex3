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

#ifndef	__COPYPROGRESS_H__
#define	__COPYPROGRESS_H__

#pragma once

#include "../framework/far/progress.h"

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

#endif//__COPYPROGRESS_H__
