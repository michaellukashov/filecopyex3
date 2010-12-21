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

#ifndef	__ENGINE_H__
#define	__ENGINE_H__

#pragma once

#include "copyprogress.h"
#include "classes.h"

struct FileStruct
{
	int Attr, RenameNum, OverMode, Flags, Level, PanelIndex,
		SectorSize;
	__int64 Size, Read, Written, ResumePos;
	FILETIME Modify;
};

struct BuffStruct
{
	int NextPos, WritePos, FileNumber;
	int EndFlag;
};

struct BuffInfo
{
	unsigned char *Buffer;
	int BuffSize;
	__int64 OrgSize;
	HANDLE OutFile;
	int OutNum;
	BuffStruct *BuffInf;
	String SrcName, DstName;
};

struct RememberStruct
{
	String Src, Dst;
	__int64 Size;
	FILETIME Modify;
	int Attr, Flags, Level;
};

class Engine
{
public:
	Engine();
	int Main(int move, int curonly);

private:
	FileNameStore SrcNames, DstNames;
	ArrayStore<FileStruct> Files;
	FileNameStoreEnum FlushSrc, FlushDst;

	int Parallel, BufSize, Streams, Rights, Move,
		CompressMode, EncryptMode, OverwriteMode,
		SkipNewer, SkippedToTemp;
	__int64 ReadSpeedLimit, WriteSpeedLimit;
	int ReadAlign;
	int _CopyDescs, _ClearROFromCD, _DescsInDirs, _ConfirmBreak,
		_HideDescs, _UpdateRODescs, _InverseBars, _PreallocMin, _UnbuffMin;
	int Aborted, LastFile, KeepFiles, FileCount, CopyCount;
	void Copy();

	BuffInfo *wbi, *bi;
	HANDLE BGThread, FlushEnd, UiFree;
	int InitBuf(BuffInfo *bi);
	void UninitBuf(BuffInfo *bi);
	void SwapBufs(BuffInfo *src, BuffInfo *dst);
	int CheckEscape(BOOL ShowKeepFilesCheckBox = TRUE);
	int AskAbort(BOOL ShowKeepFilesCheckBox = TRUE);
	int FlushBuff(BuffInfo *bi);
	void BGFlush();
	int WaitForFlushEnd();
	friend unsigned int __stdcall FlushThread(void* p);
	void FinalizeBuf(BuffInfo *bi);
	void ProcessDesc(int fnum);
	void ShowReadName(const String&);
	void ShowWriteName(const String&);
	void ShowProgress(__int64, __int64, __int64, __int64, __int64, 
		__int64, __int64, __int64);
	int CheckOverwrite(int fnum, String& ren);
	CopyProgress CopyProgressBox;
	FarProgress ScanFoldersProgressBox;
	__int64 ReadCb, WriteCb, ReadTime, WriteTime, TotalBytes,
		ReadN, WriteN, TotalN, FirstWrite, StartTime;
	void Delay(__int64, __int64, __int64&, __int64);
	int SectorSize;

	int CheckOverwrite(int, const String&, const String&, String&);
	int CheckOverwrite2(int, const String&, const String&, String&);
	void SetOverwriteMode(int);
	int AddFile(const String& _src, const String& _dst, int Attr,
		__int64 Size, FILETIME &Modify, int Flags, int Level,
		int PanelIndex=-1);
	int AddFile(const String& Src, const String& Dst, 
		WIN32_FIND_DATA &fd, int Flags, int Level,
		int PanelIndex=-1);
	void AddTopLevelDir(const String &dir, const String &dstmask, 
		int Flags, wchar_t pc);
	void RememberFile(const String& Src, const String& Dst, 
		WIN32_FIND_DATA &fd, int Flags, int Level,
		RememberStruct&);
	int AddRemembered(RememberStruct&);
	int DirStart(const String& dir, const String& dst);
	int DirEnd(const String& dir, const String& dst);
	String FindDescFile(const String& dir, int *idx=NULL);
	String FindDescFile(const String& dir, WIN32_FIND_DATA &fd, int *idx=NULL);
	void FarToWin32FindData(const FAR_FIND_DATA &fd, WIN32_FIND_DATA &wfd);
	String CurPathDesc;
	int CurPathFlags, CurPathAddFlags;
	WIN32_FIND_DATA DescFindData;

	StringList errTypes;
	int EngineError(const String& s, const String& fn, int code, int& flg,
		const String& title="", const String& type_id="");


	BOOL CheckFreeDiskSpace(const __int64 TotalBytesToProcess, const int MoveMode,
		const String& srcpathstr, const String& dstpathstr);
};

void Compress(HANDLE handle, int f);
void Encrypt(const String& fn, int f);
void CopyACL(const String& src, const String& dst);

#endif//__ENGINE_H__
