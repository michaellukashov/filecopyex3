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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.	If not, see <http://www.gnu.org/licenses/>.
*/

#include <process.h>
#if !defined(_MSC_VER)
#include <c++/bits/unique_ptr.h>
#endif
#include "Framework/DescList.h"
#include "Framework/ObjString.h"
#include "Framework/FileUtils.h"
#include "Framework/StringList.h"
#include "Framework/StrUtils.h"
#include "Common.h"
#include "Engine.h"
#include "Guid.hpp"
#include "FarPlugin.h"
#include "EngineTools.h"
#include "Tools.h"
#include "UI.h"

#define AllocAlign	65536

// Bug #10 fixed by Ivanych
//#define ReadBlock	 (4 << 20)
//#define WriteBlock	(4 << 20)

#define ReadBlock	 0x20000		 // 128 KB
#define WriteBlock	0x20000		 // 128 KB

PluginPanelItem* GetPanelItem(HANDLE hPlugin, FILE_CONTROL_COMMANDS Command, intptr_t Param1)
{
	size_t Size = Info.PanelControl(hPlugin, Command, Param1, 0);
	PluginPanelItem* item = reinterpret_cast<PluginPanelItem*>(new char[Size]);

	if (item)
	{
		FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), Size, item};
		Info.PanelControl(hPlugin, Command, Param1, &gpi);
		/*
		*Param2=*item;
		Param2->FileName=wcsdup(item->FileName);
		Param2->AlternateFileName=wcsdup(item->AlternateFileName);
		Param2->Description=NULL;
		Param2->Owner=NULL;
		Param2->CustomColumnData=NULL;
		Param2->CustomColumnNumber=0;
		Param2->UserData.Data=NULL;
		Param2->UserData.FreeData=NULL;
		free(item);
		*/
	}
	return item;
}


int warn(const String& s)
{
	return ShowMessage(LOC("CopyDialog.Warning"), s+"\n"+LOC("CopyDialog.WarnPrompt"), FMSG_MB_YESNO) == 0;
}


void ShowErrorMessage(const String& s)
{
	//ShowMessage(LOC("CopyDialog.Error"), s, FMSG_WARNING | FMSG_MB_OK);

	String msgbuf = LOC("CopyDialog.Error") + "\n" + s + "\n";

	Info.Message(&MainGuid, &UnkGuid, FMSG_WARNING | FMSG_MB_OK | FMSG_ALLINONE, NULL, (const wchar_t * const *)msgbuf.ptr(), 0, 0);

}


int64_t GetPhysMemorySize()
{
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	return ms.dwTotalPhys;
}

Engine::Engine(): FlushSrc(&SrcNames), FlushDst(&DstNames), BGThread(NULL), FlushEnd(NULL), UiFree(NULL)
{
	_CopyDescs= 0;
	_ClearROFromCD = 0;
	_DescsInDirs = 0;
	_ConfirmBreak = 0;
	_HideDescs = 0;
	_UpdateRODescs = 0;
	_InverseBars = 0;
	_PreallocMin = 0;
	_UnbuffMin = 0;
	copyCreationTime = false;
	copyLastAccessTime = false;
	copyLastWriteTime = false;
	Aborted = 0;
	LastFile = 0;
	KeepFiles = 0;
	FileCount = 0; 
	CopyCount = 0;
	wbi = NULL;
	bi = NULL;

	Parallel=Streams=Rights=Move=SkipNewer=SkippedToTemp=0;
	CompressMode=EncryptMode=ATTR_INHERIT;
	OverwriteMode=OM_PROMPT;
	BufSize=0;

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	ReadAlign=si.dwPageSize;

//	errTypes.SetOptions(slSorted | slIgnoreCase);
	SectorSize = 0;
	CurPathFlags = 0;
	CurPathAddFlags = 0;
}

int Engine::InitBuf(BuffInfo *bi)
{
	bi->OutFile=NULL;
	bi->OutNum=-1;

	bi->BuffSize=BufSize;
	if (Parallel) bi->BuffSize/=2;
	bi->BuffSize=(bi->BuffSize/AllocAlign+1)*AllocAlign;

	bi->Buffer = static_cast<UCHAR*>(Alloc(bi->BuffSize));
	if (!bi->Buffer)
	{
		Error(LOC("Error.MemAlloc"), GetLastError());
		return FALSE;
	}
	bi->BuffInf = static_cast<BuffStruct*>(Alloc(SrcNames.Count()*sizeof(BuffStruct)));
	if (!bi->BuffInf)
	{
		Free(bi->Buffer);
		Error(LOC("Error.MemAlloc"), GetLastError());
		return FALSE;
	}

	return TRUE;
}

void Engine::UninitBuf(BuffInfo *bi)
{
	Free(bi->Buffer);
	Free(bi->BuffInf);
}

void Engine::SwapBufs(BuffInfo *src, BuffInfo *dst)
{
	memcpy(dst->Buffer, src->Buffer, src->BuffSize);
	memcpy(dst->BuffInf, src->BuffInf, SrcNames.Count()*sizeof(BuffStruct));
}

// bugfixed by slst: bug #24
// Added parameter with default TRUE value
int Engine::AskAbort(BOOL ShowKeepFilesCheckBox)
{
	if (_ConfirmBreak)
	{
		WaitForSingleObject(UiFree, INFINITE);

		int flg = eeYesNo | eeOneLine;
		if (ShowKeepFilesCheckBox) flg |= eeShowKeepFiles;

		int res=EngineError(LOC("CopyError.StopPrompt"), "", 0, flg, LOC("CopyError.StopTitle"));

		HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
		FlushConsoleInputBuffer(h);
		SetEvent(UiFree);

		Aborted = (res==0);
		if (Aborted) KeepFiles=flg & eerKeepFiles;
		return Aborted;
	}
	else
	{
		KeepFiles=0;
		return 1;
	}
}

// bugfixed by slst: bug #24
// Added parameter with default TRUE value
int Engine::CheckEscape(BOOL ShowKeepFilesCheckBox)
{
	if (WaitForSingleObject(UiFree, 0) == WAIT_TIMEOUT) return FALSE;

	int escape = FALSE;
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	while (1)
	{
		INPUT_RECORD rec;
		DWORD rc;
		PeekConsoleInput(h, &rec, 1, &rc);
		if (!rc) break;
		ReadConsoleInput(h, &rec, 1, &rc);
		if (rec.EventType==KEY_EVENT)
			if (rec.Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE
				&& !(rec.Event.KeyEvent.dwControlKeyState &
				(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|LEFT_ALT_PRESSED|
				RIGHT_ALT_PRESSED|SHIFT_PRESSED))
				&& rec.Event.KeyEvent.bKeyDown)
			{
				FlushConsoleInputBuffer(h);
				escape = TRUE;
			}
	}

	SetEvent(UiFree);

	if (escape && !AskAbort(ShowKeepFilesCheckBox)) escape=0;

	return escape;
}

void Engine::FinalizeBuf(BuffInfo *bi)
{
	HANDLE Handle=bi->OutFile;
	int fnum=bi->OutNum;
	String &DstName=bi->DstName;
	String &SrcName=bi->SrcName;
	FileStruct &info=Files[fnum];

	if (!(info.Flags & FLG_SKIPPED)) {
		if (info.Flags & FLG_BUFFERED) {
			setFileTime(Handle, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
		}
	}

	Close(Handle);

	if (!(info.Flags & FLG_SKIPPED)) {
		info.Flags|=FLG_COPIED;
		LastFile=fnum;

		if (!(info.Flags & FLG_BUFFERED)) {
			if (info.OverMode == OM_APPEND) {
				setFileSizeAndTime(DstName, bi->OrgSize+info.Size, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
			} else {
				setFileSizeAndTime(DstName, info.Size, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
			}
		}

		if (Rights) CopyACL(SrcName, DstName);
		Encrypt(DstName, EncryptMode);
		SetFileAttributes(DstName.ptr(), info.Attr);

		if (Move) {
del_retry:
			// Bug #6 fixed by CDK
			if (FileExists(SrcName) && !Delete(SrcName)) {
				WaitForSingleObject(UiFree, INFINITE);
				int flg=eeRetrySkipAbort | eeAutoSkipAll;
				int res=EngineError(LOC("Error.FileDelete"), SrcName, GetLastError(), flg, "", "Error.FileDelete");
				SetEvent(UiFree);
				if (res==RES_RETRY) {
					goto del_retry;
				} else {
					if (res==RES_ABORT) {
						Aborted=1;
					}
				}
			} else {
				info.Flags |= FLG_DELETED;
			}
		} // if (Move)

		WriteN++;

		size_t dnum=fnum+1;
		while (dnum < SrcNames.Count() && Files[dnum].Flags & (FLG_DIR_POST | FLG_DIR_PRE | FLG_DESCFILE))
		{
			if (!(Files[dnum].Flags & FLG_COPIED)) {
				if (Files[dnum].Flags & FLG_DESCFILE) {
					ProcessDesc(dnum);
				} else if (Files[dnum].Flags & FLG_DIR_POST && !(Files[dnum].Flags & FLG_DIR_NOREMOVE) && Move) {
					if (RmDir(FlushSrc.GetByNum(dnum))) {
						Files[dnum].Flags |= FLG_COPIED | FLG_DELETED;
					}
				}
			}
			dnum++;
		}
	} else {
		if (!(info.Flags & FLG_DECSIZE)) {
			WriteCb -= info.Written;
			ReadCb -= info.Read;
			TotalBytes -= info.Size;
			TotalN--;
			info.Flags |= FLG_DECSIZE;
		}

		if (Handle) {
			if (info.OverMode == OM_APPEND || info.OverMode == OM_RESUME) {
				if (KeepFiles || (info.Flags & FLG_KEEPFILE)) {
					setFileSizeAndTime(DstName, bi->OrgSize+info.Written, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
				} else {
					setFileSizeAndTime(DstName, bi->OrgSize, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
				}
			} else {
				if (KeepFiles || (info.Flags & FLG_KEEPFILE)) {
					setFileSizeAndTime(DstName, info.Written, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
				} else {
					Delete(DstName);
				}
			}
			SetFileAttributes(DstName.ptr(), info.Attr);
		}
	}
}

void Engine::ProcessDesc(int fnum)
{
	String DstName=FlushDst.GetByNum(fnum);
	String SrcName=FlushSrc.GetByNum(fnum);
	//TODO: OM_RENAME can change DstName
	FileStruct &info=Files[fnum];

	String DstPath=AddEndSlash(ExtractFilePath(DstName));
	String df=FindDescFile(DstPath);
	if (!df.empty()) {
		DstName=DstPath+df;
	}

	DescList SrcList, DstList;
	SrcList.LoadFromFile(SrcName);
	int attr=GetFileAttributes(DstName.ptr());
	if (!_UpdateRODescs && attr!=0xFFFFFFFF
		&& (attr & FILE_ATTRIBUTE_READONLY)) return;
	DstList.LoadFromFile(DstName);

	bool inverse=(info.Flags & FLG_DESC_INVERSE)!=0;
	if (!inverse)
	{
		if (Move)
			SrcList.SetAllSaveFlags(0);
	}
	else
		SrcList.SetAllMergeFlags(0);

	int j=fnum-1;
	while (j>=0 && Files[j].Level>=Files[fnum].Level)
	{
		if (Files[j].Level==Files[fnum].Level)
		{
			String sn=SrcNames.GetNameByNum(j);
			String dn=DstNames.GetNameByNum(j);
			int Same=0;

			// bug #43 fixed by axxie
			if (sn.cmp(dn))
			{
				SrcList.Rename(sn, dn, 1);
				sn=dn;
				if (!SrcName.icmp(DstName)) Same=1;
			}

			if (!(Files[j].Flags & FLG_DIR_POST)
				&& ((Files[j].Flags & FLG_COPIED)!=0)==inverse)
			{
				SrcList.SetMergeFlag(sn, inverse);
			}
			if (Move && !(Files[j].Flags & FLG_DIR_PRE))
			{
				if (Same)
					SrcList.SetSaveFlag(sn, 1);
				else if (((Files[j].Flags & FLG_DELETED)!=0)==inverse)
					SrcList.SetSaveFlag(sn, !inverse);
			}
		}
		j--;
	}

	DstList.Merge(SrcList);
	if (DstList.SaveToFile(DstName))
	{
		info.Flags |= FLG_COPIED;
		if (_HideDescs)
			SetFileAttributes(DstName.ptr(),
				GetFileAttributes(DstName.ptr()) | FILE_ATTRIBUTE_HIDDEN);
		if (Move)
		{
			int attr=GetFileAttributes(SrcName.ptr());
			if (!_UpdateRODescs && attr!=0xFFFFFFFF
				&& (attr & FILE_ATTRIBUTE_READONLY)) return;
			if (!SrcList.SaveToFile(SrcName))
				Error2(LOC("Error.WriteDesc"), SrcName, GetLastError());
		}
	}
	else
	{
		Error2(LOC("Error.WriteDesc"), DstName, GetLastError());
		info.Flags |= FLG_SKIPPED | FLG_ERROR;
	}
}

int Engine::FlushBuff(BuffInfo *bi)
{
	int Pos = 0;
	int PosInStr = 0;

	while (Pos < bi->BuffSize && bi->BuffInf[PosInStr].FileNumber >= 0)
	{
		int fnum=bi->BuffInf[PosInStr].FileNumber;
		String
			SrcName=FlushSrc.GetByNum(fnum),
			DstName=FlushDst.GetByNum(fnum);
		FileStruct &info=Files[fnum];
		bi->OutNum=fnum;

		if (Aborted) info.Flags|=FLG_SKIPPED;

		if (!bi->OutFile && !(info.Flags & FLG_SKIPPED))
			{
			if (FileExists(DstName))
			{
				if (info.OverMode == OM_PROMPT)
					info.OverMode = OverwriteMode;
				if (info.OverMode == OM_RENAME && info.RenameNum)
					DstName = DupName(DstName, info.RenameNum);
				if (info.OverMode == OM_PROMPT)
				{
rep:
					String renn;
					info.OverMode = CheckOverwrite2(fnum, SrcName, DstName, renn);
					if (info.OverMode == OM_RENAME)
						DstName = renn;
				}
			}
			else info.OverMode=OM_OVERWRITE;

			if (EncryptMode==ATTR_ON) info.Attr |= FILE_ATTRIBUTE_ENCRYPTED;
			else info.Attr &= ~FILE_ATTRIBUTE_ENCRYPTED;

			bi->SrcName=SrcName;
			bi->DstName=DstName;

open_retry:
			int oflg=info.Flags & FLG_BUFFERED? OPEN_BUF: 0;
			switch (info.OverMode)
			{
				case OM_OVERWRITE:
				case OM_RENAME:
					bi->OutFile = Open(DstName, OPEN_CREATE | oflg, info.Attr); break;
				case OM_APPEND:
					info.Flags |= FLG_BUFFERED;
					oflg |= OPEN_BUF;
					bi->OutFile = Open(DstName, OPEN_APPEND | oflg); break;
				case OM_RESUME:
					bi->OutFile = Open(DstName, OPEN_WRITE | oflg);
					if (FSeek(bi->OutFile, info.ResumePos, FILE_BEGIN)==-1)
						FWError(Format(L"FSeek to %d failed, code %d", info.ResumePos,
							(int)GetLastError()));
					break;
				case OM_SKIP:
					info.Flags |= FLG_SKIPPED; break;
				case OM_PROMPT:
				case OM_CANCEL:
					if (AskAbort()) info.Flags |= FLG_SKIPPED;
					else goto rep;
					break;
			}

			ShowWriteName(DstName);

			if (!(info.Flags & FLG_SKIPPED))
			{
				if (bi->OutFile)
				{
					Compress(bi->OutFile, CompressMode);
					//Encrypt(bi->OutFile, EncryptMode);
					bi->OrgSize = FileSize(bi->OutFile);

					int64_t size=info.OverMode==OM_APPEND?bi->OrgSize+info.Size:info.Size;
					if (size>=_PreallocMin*1024
						&& GetCompression(bi->OutFile)==0)
					{
						int64_t sp;
						if (!(info.Flags & FLG_BUFFERED))
						{
							sp=size/info.SectorSize;
							if (size%info.SectorSize) sp++;
							sp*=info.SectorSize;
						}
						else sp=size;
						int64_t bp=FTell(bi->OutFile);
						FSeek(bi->OutFile, sp, FILE_BEGIN);
						SetEndOfFile(bi->OutFile);
						FSeek(bi->OutFile, bp, FILE_BEGIN);
					}
				}
				else
				{
					WaitForSingleObject(UiFree, INFINITE);
					int flg=eeRetrySkipAbort | eeAutoSkipAll,
							res=EngineError(LOC("Error.OutputFileCreate"), DstName, GetLastError(),
							flg, "", "Error.OutputFileCreate");
					SetEvent(UiFree);
					if (res==RES_RETRY) goto open_retry;
					else if (res==RES_ABORT) Aborted=1;
				}
			}
		}
		if (!(info.Flags & FLG_SKIPPED) && !bi->OutFile)
			info.Flags |= FLG_SKIPPED | FLG_ERROR;

		if (!(info.Flags & FLG_SKIPPED))
		{
			while (Pos < bi->BuffInf[PosInStr].WritePos)
			{
				if (!Parallel && CheckEscape() || Aborted)
				{
					info.Flags |= FLG_SKIPPED;
					goto skip;
				}

				int wsz = (int)Min(bi->BuffInf[PosInStr].WritePos - Pos, WriteBlock),
						wsz1 = wsz;
				if (info.Flags & FLG_BUFFERED)
					wsz = (int)Min((long long)wsz, info.Size-info.Written);
retry:
				int64_t st = GetTime();
				int k = Write(bi->OutFile, bi->Buffer+Pos, wsz);

				if (k < wsz)
				{
					WaitForSingleObject(UiFree, INFINITE);
					int flg=eeShowReopen | eeShowKeepFiles | eeRetrySkipAbort | eeAutoSkipAll,
							res=EngineError(LOC("Error.Write"), DstName, GetLastError(), flg,
							"", "Error.Write");
					SetEvent(UiFree);
					if (res==RES_RETRY)
					{
						if (flg & eerReopen)
						{
							int64_t Pos=info.Written;
							if (info.OverMode==OM_RESUME) Pos+=info.ResumePos;
							Close(bi->OutFile);
reopen_retry:
							int oflg=info.Flags & FLG_BUFFERED? OPEN_BUF: 0;
							bi->OutFile=Open(DstName, OPEN_WRITE | oflg);
							if (!bi->OutFile)
							{
								WaitForSingleObject(UiFree, INFINITE);
								int flg=eeShowKeepFiles | eeRetrySkipAbort/* | eeAutoSkipAll*/,
										res=EngineError(LOC("Error.OutputFileCreate"),
											DstName, GetLastError(), flg, "", "Error.OutputFileCreate");
								SetEvent(UiFree);
								if (res==RES_RETRY) goto reopen_retry;
								else
								{
									info.Flags |= FLG_SKIPPED | FLG_ERROR;
									if (flg & eerKeepFiles) info.Flags |= FLG_KEEPFILE;
									bi->OutFile = INVALID_HANDLE_VALUE;
									if (res==RES_ABORT) Aborted=1;
									goto skip;
								}
							}
							if (FSeek(bi->OutFile, Pos, FILE_BEGIN)==-1)
								FWError(Format(L"FSeek to %d failed, code %d", Pos,
									(int)GetLastError()));
						}
						goto retry;
					}
					else
					{
						info.Flags |= FLG_SKIPPED | FLG_ERROR;
						if (flg & eerKeepFiles) info.Flags |= FLG_KEEPFILE;
						if (res==RES_ABORT) Aborted=1;
						goto skip;
					}
				}

				int64_t wt = GetTime()-st;
				WriteTime += wt;
				info.Written += k;
				WriteCb += k;
				Pos += k;
				if (!FirstWrite) FirstWrite=GetTime()-StartTime;

				Delay(wt, k, WriteTime, WriteSpeedLimit);
				// bug #22 fixed by Axxie
				ShowWriteName(DstName);
				ShowProgress(ReadCb, WriteCb, TotalBytes, ReadTime, WriteTime, ReadN, WriteN, TotalN);

				if (wsz < wsz1)
					Pos = bi->BuffInf[PosInStr].WritePos;
			}

skip: ;
		}
		Pos = bi->BuffInf[PosInStr].NextPos;

		if (bi->BuffInf[PosInStr].EndFlag || info.Flags & FLG_SKIPPED)
		{
			FinalizeBuf(bi);
			bi->OutFile=NULL;
			bi->OutNum=-1;
			bi->SrcName="";
			bi->DstName="";
		}

		if (Aborted) break;
		PosInStr++;
	}

	if (Parallel) SetEvent(FlushEnd);
	return !Aborted;
}

unsigned int __stdcall FlushThread(void* p)
{
	Engine* eng = static_cast<Engine*>(p);
	return eng->FlushBuff(eng->wbi);
}

void Engine::BGFlush()
{
	BGThread = (HANDLE)_beginthreadex(NULL, 0, FlushThread, this, 0, NULL);
}

int Engine::WaitForFlushEnd()
{
	while (WaitForSingleObject(FlushEnd, 200) == WAIT_TIMEOUT)
	{
		if (CheckEscape()) return FALSE;
	}
	WaitForSingleObject(BGThread, INFINITE);
	CloseHandle(BGThread);
	BGThread = NULL;
	return TRUE;
}

struct CurDirInfo
{
	int SectorSize;
};

void Engine::Copy()
{
	LastFile=-1;
	Aborted=0;
	KeepFiles=0;
	ReadCb=WriteCb=ReadTime=WriteTime=ReadN=WriteN=FirstWrite=0;
	StartTime=GetTime();

	BuffInfo _bi, _wbi;
	bi = &_bi;
	wbi = &_wbi;
	if (!InitBuf(bi)) return;
	if (Parallel)
	{
		if (!InitBuf(wbi))
		{
			UninitBuf(bi);
			return;
		}
		FlushEnd = CreateEvent(NULL, FALSE, TRUE, NULL);
	}

	int BuffPos = 0;
	int FilesInBuff = 0;

	UiFree = CreateEvent(NULL, FALSE, TRUE, NULL);

	CopyProgressBox.InverseBars=_InverseBars;
	if (FileCount) CopyProgressBox.Start(Move);

	FileNameStoreEnum Src(&SrcNames);
	FileNameStoreEnum Dst(&DstNames);
	std::vector<CurDirInfo> CurDirStack;

	for (size_t i=0; i<Src.Count(); i++)
	{
		String SrcName = Src.GetNext();
		String DstName = Dst.GetNext();

		FileStruct &info=Files[i];
		if (info.Flags & FLG_SKIPPED ||
			info.Flags & FLG_COPIED ||
			info.Flags & FLG_DESCFILE) continue;

		if (info.Flags & FLG_DIR_PRE)
		{
			if (info.Flags & FLG_DIR_FORCE)
				ForceDirectories(AddEndSlash(DstName));
			else
			{
				CreateDirectory(DstName.ptr(), NULL);
				SetFileAttributes(DstName.ptr(), info.Attr);
			}
			if (!(info.Flags & FLG_TOP_DIR))
			{
					HANDLE hd = CreateFile(DstName.ptr(), GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
						OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
					if (hd != INVALID_HANDLE_VALUE)
					{
						Compress(hd, CompressMode);
						if (!(info.Flags & FLG_DIR_FORCE)) {
							setFileTime(hd, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
						}
						CloseHandle(hd);
					}
					if (EncryptMode != ATTR_INHERIT)
						Encrypt(DstName, EncryptMode);
					if (Rights && !(info.Flags & FLG_DIR_FORCE))
						CopyACL(SrcName, DstName);
			}
			info.Flags |= FLG_COPIED;

			CurDirInfo cdi;
			int dattr=GetFileAttributes(DstName.ptr());
			if (!CurDirStack.size() || dattr != 0xFFFFFFFF && dattr & FILE_ATTRIBUTE_REPARSE_POINT) {
				cdi.SectorSize = GetSectorSize(DstName);
			} else {
				cdi.SectorSize = CurDirStack.back().SectorSize;
				CurDirStack.pop_back();
			}
			CurDirStack.push_back(cdi);
			SectorSize=cdi.SectorSize;
			continue;
		} // if (info.Flags & FLG_DIR_PRE)

		if (info.Flags & FLG_DIR_POST) {
			CurDirStack.pop_back();
			if(CurDirStack.size()) {
				SectorSize=CurDirStack[CurDirStack.size()-1].SectorSize;
			}
			continue;
		}

open_retry:
		HANDLE InputFile = Open(SrcName, OPEN_READ);
		ShowReadName(SrcName);

		if (!InputFile)
		{
			WaitForSingleObject(UiFree, INFINITE);
			int flg=eeRetrySkipAbort | eeAutoSkipAll,
					res=EngineError(LOC("Error.InputFileOpen"), SrcName, GetLastError(),
					flg, "", "Error.InputFileOpen");
			SetEvent(UiFree);
			if (res==RES_RETRY) goto open_retry;
			else
			{
				info.Flags |= FLG_SKIPPED | FLG_ERROR;
				if (res==RES_ABORT) goto abort;
				else continue;
			}
		}

		if (OverwriteMode == OM_RESUME)
		{
			if (FSeek(InputFile, info.ResumePos, FILE_BEGIN)==-1)
				FWError(Format(L"FSeek to %d failed, code %d", info.ResumePos,
					(int)GetLastError()));
		}

		while (1)
		{
			if (info.Flags & FLG_SKIPPED) break;
			info.SectorSize=SectorSize;
			if (info.Size < _UnbuffMin*1024)
				info.Flags |= FLG_BUFFERED;

			while (BuffPos < bi->BuffSize)
			{
				if (info.Flags & FLG_SKIPPED) break;
				int j, cb = Min(ReadBlock, bi->BuffSize-BuffPos);
retry:
				int64_t st = GetTime();
				j = Read(InputFile, bi->Buffer+BuffPos, cb);

				if (j==-1)
				{
					WaitForSingleObject(UiFree, INFINITE);
					int flg=eeShowReopen | eeShowKeepFiles | eeRetrySkipAbort | eeAutoSkipAll,
							res=EngineError(LOC("Error.Read"), SrcName, GetLastError(), flg,
							"", "Error.Read");
					SetEvent(UiFree);
					if (res==RES_RETRY)
					{
						if (flg & eerReopen)
						{
							int64_t Pos=info.Read;
							if (info.OverMode==OM_RESUME) Pos+=info.ResumePos;
							Close(InputFile);
reopen_retry:
							InputFile=Open(SrcName, OPEN_READ);
							if (!InputFile)
							{
								WaitForSingleObject(UiFree, INFINITE);
								int flg=eeShowKeepFiles | eeRetrySkipAbort/* | eeAutoSkipAll*/,
										res=EngineError(LOC("Error.InputFileOpen"), SrcName,
											GetLastError(), flg, "", "Error.InputFileOpen");
								SetEvent(UiFree);
								if (res==RES_RETRY) goto reopen_retry;
								else
								{
									info.Flags |= FLG_SKIPPED | FLG_ERROR;
									if (flg & eerKeepFiles) info.Flags |= FLG_KEEPFILE;
									if (res==RES_ABORT) goto abort;
									else goto skip;
								}
							}
							if (FSeek(InputFile, Pos, FILE_BEGIN)==-1)
								FWError(Format(L"FSeek to %d failed, code %d", Pos,
									(int)GetLastError()));
						}
						goto retry;
					}
					else
					{
						info.Flags |= FLG_SKIPPED | FLG_ERROR;
						if (flg & eerKeepFiles) info.Flags |= FLG_KEEPFILE;
						if (res==RES_ABORT) goto abort;
						else goto skip;
					}
				} // if (j==-1)

				int64_t rt = GetTime()-st;
				ReadTime += rt;
				info.Read += j;
				ReadCb += j;
				BuffPos += j;

				Delay(rt, j, ReadTime, ReadSpeedLimit);
				ShowProgress(ReadCb, WriteCb, TotalBytes, ReadTime, WriteTime, ReadN, WriteN, CopyCount);
				// bug #22 fixed by Axxie
				ShowReadName(SrcName);

				if (CheckEscape()) goto abort;
				if (j<cb) break;
			} // while (BuffPos < bi->BuffSize)

skip:
			int abp=BuffPos;
			if (abp%SectorSize) abp=(abp/SectorSize+1)*SectorSize;
			bi->BuffInf[FilesInBuff].WritePos = abp;
			if (BuffPos%ReadAlign) BuffPos=(BuffPos/ReadAlign+1)*ReadAlign;
			bi->BuffInf[FilesInBuff].NextPos = BuffPos;
			bi->BuffInf[FilesInBuff].FileNumber = i;
			if (BuffPos == bi->BuffSize)
			{
				bi->BuffInf[FilesInBuff].EndFlag = 0;
				if (!Parallel)
				{
					if (!FlushBuff(bi)) goto abort;
				}
				else
				{
					if (!WaitForFlushEnd()) goto abort;
					SwapBufs(bi, wbi);
					BGFlush();
				}
				BuffPos = 0;
				FilesInBuff = 0;
			}
			else
			{
				bi->BuffInf[FilesInBuff].EndFlag = 1;
				FilesInBuff++;
				break;
			}
		} // while (1)

		Close(InputFile);
		ReadN++;
		continue;

abort:
		info.Flags |= FLG_SKIPPED;
		Close(InputFile);
		Aborted=1;
		break;
	} // for (int i=0; i<Src.Count(); i++)

	if (Parallel)
	{
		while (!WaitForFlushEnd());
		bi->OutFile=wbi->OutFile;
		bi->OutNum=wbi->OutNum;
		bi->SrcName=wbi->SrcName;
		bi->DstName=wbi->DstName;
	}
	bi->BuffInf[FilesInBuff].FileNumber = -1;
	if (!Aborted) FlushBuff(bi);
	else if (bi->OutNum!=-1) FinalizeBuf(bi);

	if (Parallel)
	{
		UninitBuf(wbi);
		CloseHandle(FlushEnd);
	}
	UninitBuf(bi);

	if (FileCount) CopyProgressBox.Stop();
	else
	{
		FileNameStoreEnum Src(&SrcNames);
		for (size_t i=0; i<Src.Count(); i++)
		{
			if (!(Files[i].Flags & FLG_COPIED))
			{
				if (Files[i].Flags & FLG_DESCFILE)
					ProcessDesc(i);
				else if (Move && Files[i].Flags & FLG_DIR_POST
					&& !(Files[i].Flags & FLG_DIR_NOREMOVE))
				{
					if (RmDir(Src.GetByNum(i)))
						Files[i].Flags |= FLG_DELETED;
				}
			}
		}
	}

	CloseHandle(UiFree);
}

void Engine::ShowReadName(const String& fn)
{
	if (WaitForSingleObject(UiFree, 0) == WAIT_TIMEOUT) return;
	CopyProgressBox.ShowReadName(fn);
	SetEvent(UiFree);
}

void Engine::ShowWriteName(const String& fn)
{
	if (WaitForSingleObject(UiFree, 0) == WAIT_TIMEOUT) return;
	CopyProgressBox.ShowWriteName(fn);
	SetEvent(UiFree);
}

void Engine::ShowProgress(int64_t read, int64_t write, int64_t total,
	int64_t readTime, int64_t writeTime,
	int64_t readN, int64_t writeN, int64_t totalN)
{
	if (WaitForSingleObject(UiFree, 0) == WAIT_TIMEOUT) return;

	CopyProgressBox.ShowProgress(read, write, total, readTime, writeTime,
																readN, writeN, totalN, Parallel, FirstWrite,
																StartTime, BufSize);
	SetEvent(UiFree);
}

int Engine::CheckOverwrite2(int fnum, const String& src, const String& dst, String& ren)
{
	WaitForSingleObject(UiFree, INFINITE);
	int res=CheckOverwrite(fnum, src, dst, ren);
	SetEvent(UiFree);
	return res;
}

void Engine::Delay(int64_t time, int64_t cb, int64_t& counter, int64_t limit)
{
	if (limit<=0) return;
	int64_t wait=(int64_t)((double)cb/limit*TicksPerSec())-time;
	if (wait>0)
	{
		Sleep((int)((double)wait/TicksPerSec()*1000));
		counter+=wait;
	}
}

void Engine::FarToWin32FindData(const TPanelItem &tpi, WIN32_FIND_DATA &wfd)
{
	wfd.dwFileAttributes = tpi->FileAttributes;
	wfd.ftCreationTime = tpi->CreationTime;
	wfd.ftLastAccessTime = tpi->LastAccessTime;
	wfd.ftLastWriteTime = tpi->LastWriteTime;
	LARGE_INTEGER x;
	x.QuadPart = tpi->FileSize;
	wfd.nFileSizeHigh=x.HighPart;
	wfd.nFileSizeLow=x.LowPart;
}

String Engine::FindDescFile(const String& dir, int *idx)
{
	for (size_t i=0; i < plugin->Descs().Count(); i++) {
		if (FileExists(AddEndSlash(dir)+plugin->Descs()[i]))
		{
			if (idx) *idx = i;
			return plugin->Descs()[i];
		}
	}
	if (idx) {
		*idx = -1;
	}
	return "";
}

String Engine::FindDescFile(const String& dir, WIN32_FIND_DATA &fd, int *idx)
{
	HANDLE hf;
	for (size_t i=0; i < plugin->Descs().Count(); i++)
		if((hf=FindFirstFile((AddEndSlash(dir) + plugin->Descs()[i]).ptr(), &fd)) != INVALID_HANDLE_VALUE)
		{
			FindClose(hf);
			if (idx) *idx = i;
			return plugin->Descs()[i];
		}
	if (idx) *idx = -1;
	return "";
}

void Engine::AddTopLevelDir(const String &dir, const String &dstMask, int flags, FileName::Direction d)
{
	HANDLE hf;
	WIN32_FIND_DATA fd;

	SrcNames.AddRel(d, dir);
	DstNames.AddRel(d, ExtractFilePath(ApplyFileMaskPath(dir+"\\somefile.txt", dstMask)));

	FileStruct info;
	memset(&info, 0, sizeof(info));
	info.Flags = flags;
	info.Level = 0;
	info.PanelIndex = -1;
	if ((hf=FindFirstFile(dir.ptr(), &fd)) !=INVALID_HANDLE_VALUE)
	{
		FindClose(hf);
		info.Attr = fd.dwFileAttributes;
		info.creationTime = fd.ftCreationTime;
		info.lastAccessTime = fd.ftLastAccessTime;
		info.lastWriteTime = fd.ftLastWriteTime;
	}
	Files.push_back(info);
}

int Engine::DirStart(const String& dir, const String& dstMask)
{
	if(_CopyDescs) {
		CurPathDesc=FindDescFile(dir, DescFindData);
	}
	CurPathFlags=CurPathAddFlags=0;
	if(_ClearROFromCD && VolFlags(dir) & VF_CDROM) {
		CurPathAddFlags|=AF_CLEAR_RO;
	}
	AddTopLevelDir(dir, dstMask, FLG_DIR_PRE | FLG_DIR_FORCE | FLG_TOP_DIR | CurPathFlags, FileName::levelPlus);
	return TRUE;
}

int Engine::DirEnd(const String& dir, const String& dstMask)
{
	if(_CopyDescs && CurPathDesc != "")
	{
		if (!AddFile(dir+"\\"+CurPathDesc,
			AddEndSlash(ExtractFilePath(ApplyFileMaskPath(dir+"\\"+CurPathDesc, dstMask)))+CurPathDesc, DescFindData, AF_DESCFILE | AF_DESC_INVERSE | CurPathAddFlags, 1)
		) {
			return FALSE;
		}
	}
	AddTopLevelDir(dir, dstMask, FLG_DIR_POST | FLG_DIR_NOREMOVE | CurPathFlags, FileName::levelStar);
	return TRUE;
}

String getPanelDir(HANDLE h_panel) {
  size_t bufSize = 512; // initial size

  std::unique_ptr<unsigned char[]> buf(new unsigned char[bufSize]);
  reinterpret_cast<FarPanelDirectory*>(buf.get())->StructSize = sizeof(FarPanelDirectory);
  size_t size = Info.PanelControl(h_panel, FCTL_GETPANELDIRECTORY, bufSize, buf.get());
  if (size > bufSize) {
    bufSize = size;
    buf.reset(new unsigned char[bufSize]);
    reinterpret_cast<FarPanelDirectory*>(buf.get())->StructSize = sizeof(FarPanelDirectory);
    size = Info.PanelControl(h_panel, FCTL_GETPANELDIRECTORY, bufSize, buf.get());
  }
  //CHECK(size >= sizeof(FarPanelDirectory) && size <= buf_size);
  return reinterpret_cast<FarPanelDirectory*>(buf.get())->Name;
}

Engine::MResult Engine::Main(int move, int curOnly)
{
	PropertyMap& Options = plugin->Options();

	_CopyDescs		= Options["CopyDescs"];
	_DescsInDirs	= Options["DescsInSubdirs"];
	_ConfirmBreak	= Options["ConfirmBreak"];
	_PreallocMin	= Options["PreallocMin"];
	_UnbuffMin		= Options["UnbuffMin"];

	_ClearROFromCD	= 1; //YYY Info.AdvControl(&MainGuid, ACTL_GETSYSTEMSETTINGS, NULL) & FSS_CLEARROATTRIBUTE;
	_HideDescs = 0; //YYY Info.AdvControl(Info.ModuleNumber, ACTL_GETDESCSETTINGS, NULL) & FDS_SETHIDDEN;
	_UpdateRODescs	= 0; //YYY Info.AdvControl(Info.ModuleNumber, ACTL_GETDESCSETTINGS, NULL) & FDS_UPDATEREADONLY;

	FarDialog& dlg = plugin->Dialogs()["CopyDialog"];
	dlg.ResetControls();
	FarDialog& advdlg = plugin->Dialogs()["AdvCopyDialog"];
	advdlg.ResetControls();

	String prompt, srcPath, dstPath;
	Move = move;
	int allowPlug=0, adv=0;

	String curDir = getPanelDir(PANEL_PASSIVE);

	PanelInfo pi;
	pi.StructSize = sizeof(PanelInfo);
	Info.PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, &pi); // !!! check result!
	// fixed by Nsky: bug #40
//	_toansi(pi.CurDir);

	if ((pi.Flags&PFLAGS_PLUGIN) == PFLAGS_PLUGIN )
	{
		dstPath="plugin:";
		allowPlug=1;
	} else {
		if (pi.PanelType==PTYPE_QVIEWPANEL || pi.PanelType==PTYPE_INFOPANEL ||	!(pi.Flags & PFLAGS_VISIBLE)) {
			dstPath = "";
		} else {
			dstPath=AddEndSlash(curDir);
		}
	}

	curDir = getPanelDir(PANEL_ACTIVE);
	Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &pi); // !!! check result!

	// fixed by Nsky: bug #40
//	_toansi(pi.CurDir);
	if(pi.PanelType==PTYPE_QVIEWPANEL || pi.PanelType== PTYPE_INFOPANEL || !pi.ItemsNumber) {
		return MRES_NONE;
	}
	// Bug #5 fixed by CDK
	if ((pi.Flags & PFLAGS_REALNAMES) == 0) return MRES_STDCOPY;
	if (pi.SelectedItemsNumber > 1 && !curOnly) {
		if (move) {
			prompt=LOC("CopyDialog.MoveFilesTo");
		} else {
			prompt=LOC("CopyDialog.CopyFilesTo");
		}
	} else	{
		wchar_t buf[MAX_FILENAME];
		bool pit_sel = pi.SelectedItemsNumber && !curOnly;
		TPanelItem pit(pit_sel ? 0 : pi.CurrentItem, true, pit_sel);

		wcsncpy_s(buf, MAX_FILENAME, pit->FileName, MAX_FILENAME);
		// _toansi(buf);
		String fn=ExtractFileName(buf);
		if (fn==L"..") return MRES_NONE;
		String fmt;
		if (!move) {
			fmt=LOC("CopyDialog.CopyTo");
		} else {
			fmt=LOC("CopyDialog.MoveTo");
		}
		prompt = Format(fmt.ptr(), fn.ptr());
		if (curOnly || dstPath.empty()) dstPath=fn;
		/*else if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			&& dstpath!="queue:" && dstpath!="plugin:")
				dstpath=AddEndSlash(dstpath)+fn;*/

	}
	srcPath=CutEndSlash(curDir);

	_InverseBars=(bool)Options["ConnectLikeBars"] && pi.PanelRect.left>0;

	if (move) {
		dlg("Title")=LOC("CopyDialog.Move");
	} else {
		dlg("Title")=LOC("CopyDialog.Copy");
	}
	dlg["Label1"]("Text") = prompt;
	dlg["DestPath"]("Text") = dstPath;

	// axxie: Reset cp in case when user pressed Shift-F6/Shift-F5
	int cp = (!curOnly && srcPath!="") ? CheckParallel(srcPath, dstPath) : FALSE;
	if (!Options["AllowParallel"]) {
		dlg["ParallelCopy"]("Selected")=FALSE;
	} else {
		if(cp==1) {
			dlg["ParallelCopy"]("Selected")=TRUE;
		} else {
			if (cp==0) {
				dlg["ParallelCopy"]("Selected")=FALSE;
			} else {
				dlg["ParallelCopy"]("Selected")=Options["DefParallel"];
			}
		}
	}
	dlg["Ask"]("Selected") = !Options["OverwriteDef"];
	dlg["Overwrite"]("Selected") = Options["OverwriteDef"];

	if (WinNT) {
		advdlg["Streams"]("Selected") = Options["CopyStreamsDef"];
		advdlg["Rights"]("Selected") = Options["CopyRightsDef"];
	} else {
		advdlg["Streams"]("Disable") = 1;
		advdlg["Rights"]("Disable") = 1;
		advdlg["Compress"]("Disable") = 1;
	}
	if (!Win2K) {
		advdlg["Encrypt"]("Disable") = 1;
	}
	advdlg["creationTime"]("Selected") = Options["copyCreationTime"];
	advdlg["lastAccessTime"]("Selected") = Options["copyLastAccessTime"];
	advdlg["lastWriteTime"]("Selected") = Options["copyLastWriteTime"];
	advdlg["Compress"]("Selected") = Options["compressDef"];
	advdlg["Encrypt"]("Selected") = Options["encryptDef"];
	advdlg["ReadSpeedLimit"]("Selected") = Options["readSpeedLimitDef"];
	advdlg["ReadSpeedLimitValue"]("Text") = Options["readSpeedLimitValueDef"];
	advdlg["WriteSpeedLimit"]("Selected") = Options["writeSpeedLimitDef"];
	advdlg["WriteSpeedLimitValue"]("Text") = Options["writeSpeedLimitLimitDef"];

rep:

	int dres = dlg.Execute();

	switch (dres) {
		case 2: {
			plugin->Config();
			goto rep;
		}

		case 1: {
			int advRes = advdlg.Execute();

			if (advRes == 1) {
				Options["copyCreationTime"] = advdlg["creationTime"]("Selected");
				Options["copyLastAccessTime"] = advdlg["lastAccessTime"]("Selected");
				Options["copyLastWriteTime"] = advdlg["lastWriteTime"]("Selected");
				Options["compressDef"] = advdlg["Compress"]("Selected");
				Options["encryptDef"] = advdlg["Encrypt"]("Selected");
				Options["readSpeedLimitDef"] = advdlg["ReadSpeedLimit"]("Selected");
				Options["readSpeedLimitValueDef"] = advdlg["ReadSpeedLimitValue"]("Text");
				Options["writeSpeedLimitDef"] = advdlg["WriteSpeedLimit"]("Selected");
				Options["writeSpeedLimitLimitDef"] = advdlg["WriteSpeedLimitValue"]("Text");
				plugin->SaveOptions();
			}

			if (advRes == 0 || advRes == 1)	{
				adv=1;
				bool resume=advdlg["ResumeFiles"]("Selected");
				dlg["Label2"]("Disable")=resume;
				dlg["Ask"]("Disable")=resume;
				dlg["Skip"]("Disable")=resume;
				dlg["Overwrite"]("Disable")=resume;
				dlg["Append"]("Disable")=resume;
				dlg["Rename"]("Disable")=resume;
				dlg["SkipIfNewer"]("Disable")=resume;
				if (resume) {
					dlg["SkipIfNewer"]("Selected")=0;
				}
			}
			goto rep;
		}
		case -1: {
			return MRES_NONE;
		}
	}

	String tmpDstText=dlg["DestPath"]("Text");

	// bugfixed by slst:
	String dstText = tmpDstText.trim().trimquotes();

	// fixed by Nsky: bug #28
	if(!dstText.left(4).icmp("nul\\"))
		dstText = L"nul";

	// bugfixed by slst: bug #7
	dstText = dstText.replace("\"", "");

	if (dstText == L"plugin:")
	{
		if(allowPlug) {
			return MRES_STDCOPY_RET;
		} else {
			ShowMessage(dlg("Text"), LOC("CopyDialog.InvalidPath"), FMSG_MB_OK);
			goto rep;
		}
	}

	String relDstPath = ExpandEnv(dstText.replace("/", "\\"));
	dstPath = "";

	wchar_t CurrentDir[MAX_FILENAME];
	wchar_t dstbuf[MAX_FILENAME];
	memset(dstbuf, 0, sizeof(dstbuf));
	// Get absolute path for relative dstpath
	FSF.GetCurrentDirectory(MAX_FILENAME, CurrentDir);
	// srcpath.ptr() for temporary file panel is empty
	// Current directory is set by Far to file path of selected file
	BOOL SCDResult = SetCurrentDirectory(srcPath.ptr());
	// fixed by Nsky: bug #28
	if (relDstPath.icmp("nul") != 0) {
		dstPath = convertPath(CPM_REAL, relDstPath);
	} else {
		dstPath = relDstPath;
	}
	if (SCDResult) {
		SetCurrentDirectory(CurrentDir);
	}

	CompressMode = EncryptMode = ATTR_INHERIT;
	Streams = Rights = FALSE;

	if (WinNT)
	{
		Rights = advdlg["Rights"]("Selected");
		Streams = advdlg["Streams"]("Selected");
		CompressMode = advdlg["Compress"]("Selected");
		if(Win2K)
		{
			EncryptMode = advdlg["Encrypt"]("Selected");
			if(EncryptMode!=ATTR_INHERIT) CompressMode=ATTR_INHERIT;
		}
	}

	copyCreationTime = advdlg["creationTime"]("Selected");
	copyLastAccessTime = advdlg["lastAccessTime"]("Selected");
	copyLastWriteTime = advdlg["lastWriteTime"]("Selected");

	Parallel = dlg["ParallelCopy"]("Selected");
	SkipNewer = dlg["SkipIfNewer"]("Selected");
	SkippedToTemp = advdlg["SkippedToTemp"]("Selected");
	ReadSpeedLimit = WriteSpeedLimit=0;
	if((bool)advdlg["ReadSpeedLimit"]("Selected")) {
		ReadSpeedLimit=(int)advdlg["ReadSpeedLimitValue"]("Text")*1024;
	}
	if((bool)advdlg["WriteSpeedLimit"]("Selected")) {
		WriteSpeedLimit=(int)advdlg["WriteSpeedLimitValue"]("Text")*1024;
	}

	OverwriteMode=OM_PROMPT;
	if (advdlg["ResumeFiles"]("Selected")) {
		OverwriteMode=OM_RESUME;
	} else if(dlg["Overwrite"]("Selected")) {
		OverwriteMode=OM_OVERWRITE;
	} else if(dlg["Skip"]("Selected")) {
		OverwriteMode=OM_SKIP;
	} else if(dlg["Append"]("Selected")) {
		OverwriteMode=OM_APPEND;
	} else if(dlg["Rename"]("Selected")) {
		OverwriteMode=OM_RENAME;
	};

	// fixed by Nsky: bug #28
	if(!dstPath.icmp("nul")) {
		OverwriteMode=OM_OVERWRITE;
		CompressMode=EncryptMode=ATTR_INHERIT;
		Streams=Rights=0;
	} else {
		int vf=VolFlags(dstPath);
		if(vf!=-1)
		{
			if(!(vf & VF_STREAMS) && !adv) Streams=0;
			if(!(vf & VF_RIGHTS) && !adv) Rights=0;
			if(!(vf & VF_COMPRESSION) && !adv) CompressMode=ATTR_INHERIT;
			if(!(vf & VF_ENCRYPTION) && !adv) EncryptMode=ATTR_INHERIT;

			if(vf & VF_READONLY)
				if(!warn(LOC("CopyDialog.ReadOnlyWarn"))) goto rep;
			if(!(vf & VF_COMPRESSION) && CompressMode==ATTR_ON)
			{
				if(!warn(LOC("CopyDialog.CompressWarn"))) goto rep;
				CompressMode=ATTR_INHERIT;
			}
			if(!(vf & VF_ENCRYPTION) && EncryptMode==ATTR_ON)
			{
				if(!warn(LOC("CopyDialog.EncryptWarn"))) goto rep;
				EncryptMode=ATTR_INHERIT;
			}
			if(!(vf & VF_STREAMS) && Streams)
			{
				if(!warn(LOC("CopyDialog.StreamsWarn"))) goto rep;
				Streams=0;
			}
			if(!(vf & VF_RIGHTS) && Rights==ATTR_ON)
			{
				if(!warn(LOC("CopyDialog.RightsWarn"))) goto rep;
				Rights=0;
			}
		}
		else
		{
			//if(!warn(LOC("CopyDialog.InvalidPathWarn")))
			ShowErrorMessage(LOC("CopyDialog.InvalidPathWarn"));
			goto rep;
		}
	}

	// bugfixed by slst: bug #14
	// Check if srcpath=dstpath
	if(GetRealFileName(srcPath) == GetRealFileName(dstPath))
	{
		ShowErrorMessage(LOC("CopyDialog.OntoItselfError"));
		return MRES_NONE;
	}

	TotalBytes=0;
	TotalN=0;
	FileCount=CopyCount=0;
	int64_t Start=GetTime();

	// bugfixed by slst: bug #24
	//progress.ShowMessage(LOC("Status.CreatingList"));
	ScanFoldersProgressBox.ShowScanProgress(LOC("Status.ScanningFolders"));

	size_t curItem = curOnly;
	if (!curItem) {
		curItem = !(TPanelItem(0, true, true)->Flags & PPIF_SELECTED);
	}

	std::vector<size_t> sortIndex;
	if (curItem) {
		sortIndex.push_back(pi.CurrentItem);
	} else 	{
		for (size_t i = 0; i < pi.ItemsNumber; i++) {
			if (TPanelItem(i)->Flags & PPIF_SELECTED) {
				sortIndex.push_back(i);
			}
		}
	}

	String curPath;
	int haveCurPath=0;

	// bugfixed by slst: bug #23
	for (size_t ii=0; ii < sortIndex.size(); ii++)
	{
		size_t i = sortIndex[ii];
		TPanelItem pit(i);
		String file = pit->FileName;
		if (file == L"..") {
			continue;
		}

		String filePath=ExtractFilePath(file);
		if ( !haveCurPath || filePath.icmp(curPath)) {
			if (haveCurPath && !DirEnd(!curPath.empty() ? curPath : srcPath, dstPath)) goto fin;
			curPath=filePath;
			haveCurPath=1;
			if (!DirStart(!curPath.empty() ? curPath : srcPath, dstPath)) goto fin;
		}

		if (file.find('\\') == -1) {
			file = AddEndSlash(srcPath) + file;
		}

		if(_CopyDescs && !ExtractFileName(file).icmp(CurPathDesc))
		{
			if(ShowMessageEx(LOC("CopyDialog.Warning"),
				FormatWidthNoExt(file, 50)+"\n"+
				LOC("CopyDialog.DescSelectedWarn")+"\n"+
				LOC("CopyDialog.DescSelectedWarn1"),
				LOC("Framework.No")+"\n"+LOC("Framework.Yes"), 0)!=1) {
				continue;
			} else {
				CurPathDesc="";
			}
		}

		String dst = ApplyFileMaskPath(file, dstPath);
		WIN32_FIND_DATA wfd;
		FarToWin32FindData(pit, wfd);
		if (!AddFile(file, dst, wfd, CurPathAddFlags, 1, (int)i)) {
			goto fin;
		}
	} // for (int ii=0; ii<c; ii++)

	if (haveCurPath && !DirEnd(!curPath.empty() ? curPath : srcPath, dstPath)) goto fin;

	if (OverwriteMode == OM_RESUME)
	{
		FileNameStoreEnum Enum(&DstNames);
		// bugfixed by slst: bug #24
		//progress.ShowMessage(LOC("Status.ScanningDest"));
		ScanFoldersProgressBox.ShowScanProgress(LOC("Status.ScanningFolders"));
		for (size_t i=0; i<Enum.Count(); i++) {
			if(!(Files[i].Flags & FLG_SKIPPED) & !(Files[i].Attr & FILE_ATTRIBUTE_DIRECTORY)) {
				int64_t sz=FileSize((String)Enum.GetByNum(i));
				if (sz < Files[i].Size) {
					Files[i].ResumePos = sz/ReadAlign*ReadAlign;
					TotalBytes -= Files[i].ResumePos;
				} else {
					Files[i].Flags|= FLG_SKIPPED;
					TotalBytes -= Files[i].Size;
					TotalN--;
				}
			}
		}
	} else if(OverwriteMode == OM_SKIP || OverwriteMode == OM_RENAME || SkipNewer) {
		// bugfixed by slst: bug #24
		//progress.ShowMessage(LOC("Status.ScanningDest"));
		ScanFoldersProgressBox.ShowScanProgress(LOC("Status.ScanningFolders"));
		SetOverwriteMode(0);
	}

	ScanFoldersProgressBox.Hide();

	if (Options["BufPercent"]) {
		BufSize=(int)(GetPhysMemorySize()/100*(int)Options["BufPercentVal"]);
	} else {
		BufSize=(int)Options["BufSizeVal"]*1024;
	}

	// bugfixed by slst: bug #32
	// CheckFreeDiskSpace feature added
	if (Options["CheckFreeDiskSpace"]) {
		if(!CheckFreeDiskSpace(TotalBytes, Move, srcPath, dstPath))
			return MRES_NONE; // not enough space
	}

	if(CopyCount) Copy();

	if((bool)Options["Sound"] && !Aborted)
	{
		if(GetTime()-Start > 30*60*TicksPerSec()) beep(2);
		else if(GetTime()-Start > 10*60*TicksPerSec()) beep(1);
		else if(GetTime()-Start > 30*TicksPerSec()) beep(0);
	}

fin:
	ScanFoldersProgressBox.Hide();

	if(!Move)
	{
		Info.PanelControl(PANEL_ACTIVE, FCTL_BEGINSELECTION, 0, NULL);
		for (size_t i = 0; i < Files.size(); i++)	{
			if(Files[i].PanelIndex!=-1) {
				if(Files[i].Flags & FLG_DIR_PRE) {
					int ok=1;
					size_t j=i+1;
					while (Files[j].Level>Files[i].Level) {
						if(!(Files[j].Flags & (FLG_DIR_PRE | FLG_DIR_POST | FLG_COPIED))) {
							ok=0;
							break;
						}
						j++;
					}
					if (ok) {
						Info.PanelControl(PANEL_ACTIVE, FCTL_SETSELECTION, Files[i].PanelIndex, FALSE);
					}
					i=j-1;
				} else {
					if(!(Files[i].Flags & FLG_DIR_POST) && Files[i].Flags & FLG_COPIED)	{
						Info.PanelControl(PANEL_ACTIVE, FCTL_SETSELECTION, Files[i].PanelIndex, FALSE);
					}
				}
			}
		}
		Info.PanelControl(PANEL_ACTIVE, FCTL_ENDSELECTION, 0, NULL);
		// fixed by Nsky: bug #40
	} else {
		// Bugfixed by slst: bug #2
		Info.PanelControl(PANEL_ACTIVE, FCTL_UPDATEPANEL, 1, NULL);
		Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &pi);
		// fixed by Nsky: bug #40
		PanelRedrawInfo rpi;
		rpi.TopPanelItem = pi.TopPanelItem;

		String NewFileName;
		for (size_t idx=0; idx<Files.size(); idx++) {
			if (Files[idx].PanelIndex == pi.CurrentItem) {
				NewFileName = DstNames.GetNameByNum(idx);
				break;
			}
		}
		NewFileName = NewFileName.toLower();

		for (size_t i=0; i<pi.ItemsNumber; i++) {
			TPanelItem pit(i);
			String NewPanelFilename = pit->FileName;
			NewPanelFilename = NewPanelFilename.toLower();
			if(NewFileName == NewPanelFilename)	{
				rpi.CurrentItem = i;
				Info.PanelControl(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, &rpi);
				// fixed by Nsky: bug #40
				break;
			}
		}
	}

	Info.PanelControl(PANEL_ACTIVE, FCTL_UPDATEPANEL, 1, NULL);
	Info.PanelControl(PANEL_PASSIVE, FCTL_UPDATEPANEL, 1, NULL);
	Info.PanelControl(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, NULL);
	Info.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, NULL);
	// fixed by Nsky: bug #40

	return MRES_NONE;
}

int Engine::AddFile(const String& Src, const String& Dst, WIN32_FIND_DATA &fd, int Flags, int Level, int PanelIndex)
{
	return AddFile(Src, Dst, fd.dwFileAttributes, MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh), fd.ftCreationTime, fd.ftLastAccessTime, fd.ftLastWriteTime, Flags, Level, PanelIndex);
}

int Engine::AddFile(const String& _src, const String& _dst, int attr, int64_t size, const FILETIME& creationTime, const FILETIME& lastAccessTime, const FILETIME& lastWriteTime, int flags, int Level, int PanelIndex)
{
	// bugfixed by slst: bug #23
	if (CheckEscape(FALSE)) {
		return FALSE;
	}

	if (attr==0xFFFFFFFF) {
		return TRUE;
	}

	String src(_src);
	String dst(_dst);

	// bugfixed by slst:
	// Get here the real file names
	// unfold symlinks in file paths (if any)
	// src path
//	DWORD _src_attr = GetFileAttributes(ExtractFilePath(_src).ptr());
//	if((_src_attr != 0xFFFFFFFF) &&
//					(_src_attr & FILE_ATTRIBUTE_DIRECTORY)	&&
//					(_src_attr & FILE_ATTRIBUTE_REPARSE_POINT))
//			src = AddEndSlash(GetRealFileName(ExtractFilePath(_src))) + ExtractFileName(_src);
//	else
//			src = _src;
//
//	// dst path
//	DWORD _dst_attr = GetFileAttributes(ExtractFilePath(_dst).ptr());
//	if((_dst_attr != 0xFFFFFFFF) &&
//			(_dst_attr & FILE_ATTRIBUTE_DIRECTORY)	&&
//			(_dst_attr & FILE_ATTRIBUTE_REPARSE_POINT))
//			dst = AddEndSlash(GetRealFileName(ExtractFilePath(_dst))) + ExtractFileName(_dst);
//	else
//			dst = _dst;
	//////////////////////////////////////////////////////

	if (src==dst && !(flags & AF_DESCFILE)) {
		return TRUE;
	}

	if (flags & AF_CLEAR_RO) {
		attr &= ~FILE_ATTRIBUTE_READONLY;
	}

	int64_t sz1 = (attr & FILE_ATTRIBUTE_DIRECTORY)? 0 : size;

	FileStruct fs;
	memset(&fs, 0, sizeof(fs));
	std::vector<FileStruct>::iterator info = Files.insert(Files.end(), fs);

	info->Size = sz1;
	info->Attr = attr;
	info->creationTime = creationTime;
	info->lastAccessTime = lastAccessTime;
	info->lastWriteTime = lastWriteTime;

	info->Level = Level;
	info->PanelIndex = PanelIndex;

	FileName::Direction d;
	if (attr & FILE_ATTRIBUTE_DIRECTORY) {
		info->Flags |= FLG_DIR_PRE;
		d = FileName::levelPlus;
	} else {
		d = FileName::levelSame;
	}

	SrcNames.AddRel(d, ExtractFileName(src));
	DstNames.AddRel(d, ExtractFileName(dst));

	if (flags & AF_DESCFILE) {
		info->Flags |= FLG_DESCFILE;
		if	(flags & AF_DESC_INVERSE) {
			info->Flags |= FLG_DESC_INVERSE;
		}
		CopyCount++;
		return TRUE;
	}

	int owmode = OM_PROMPT;
	if (!(flags & AF_STREAM)) {
		if (Move) {
	retry:
			if(MoveFile(src, dst, FALSE)) {
				info->Flags |= FLG_COPIED | FLG_DELETED;
				goto fin;
			}
			int err = GetLastError();
			if (err == ERROR_ALREADY_EXISTS && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
				if(OverwriteMode == OM_RESUME) {
					if(FileSize(dst)>=info->Size) {
						info->Flags |= FLG_SKIPPED;
						goto fin;
					}
				} else {
					String ren;
					int res, j = 0;
					if (SkipNewer && Newer(dst, lastWriteTime)){
						res = OM_SKIP;
					}
					else if(OverwriteMode == OM_PROMPT) {
						res = CheckOverwrite(-1, src, dst, ren);
					} else {
						res = OverwriteMode;
					}
					switch (res)
					{
						case OM_SKIP:
							info->Flags |= FLG_SKIPPED;
							goto fin;
						break;

						case OM_OVERWRITE:
							owmode = OM_OVERWRITE;
							if(MoveFile(src, dst, TRUE)) {
								info->Flags |= FLG_COPIED | FLG_DELETED;
								goto fin;
							}
						break;

						case OM_APPEND:
							owmode = OM_APPEND;
						break;

						case OM_RENAME:
							if(OverwriteMode != OM_RENAME) {
								dst=ren;
							} else {
								while (ExistsN(dst, j)) j++;
								dst=DupName(dst, j);
							}
							goto retry;
						break;

						case OM_CANCEL:
							return FALSE;
						break;
					} // switch (res)
				}
			} // if(err == ERROR_ALREADY_EXISTS && !(attr & FILE_ATTRIBUTE_DIRECTORY))
		} // if(Move)
	} // if(!(flags & AF_STREAM))

	CopyCount++;
	TotalN++;
	if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
		FileCount++;
	}
	TotalBytes += sz1;
	info->OverMode = owmode;

	// bugfixed by slst: bug #24
	// Updates folder scan progress dialog
	ScanFoldersProgressBox.SetScanProgressInfo(TotalN, TotalBytes);

	if(!(flags & AF_STREAM))
	{
		if(attr & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(attr & FILE_ATTRIBUTE_REPARSE_POINT)
			{
				if(_ClearROFromCD && VolFlags(src) & VF_CDROM)
					flags |= AF_CLEAR_RO;
			}
			WIN32_FIND_DATA fd;
			HANDLE hf;
			if((hf = FindFirstFile((src+"\\*.*").ptr(), &fd)) != INVALID_HANDLE_VALUE)
			{
				int descidx=-1;
				RememberStruct Remember;
				while (1)
				{
					if(wcscmp(fd.cFileName, L"..") && wcscmp(fd.cFileName, L"."))
					{
						int idx;
						if(_CopyDescs && _DescsInDirs
							&& !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							&& (idx=plugin->Descs().Find(fd.cFileName))!=-1)
						{
							if(descidx=-1 || idx<descidx)
							{
								RememberFile(src+L"\\"+fd.cFileName, dst+L"\\"+fd.cFileName,
									fd, flags | AF_DESCFILE, Level+1, Remember);
								descidx=idx;
							}
						}
						else
						{
							// bugfixed by slst:
							// check for recursive symlinks
							// recursive symlinks results in stack overflow :(
							if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
											(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
							{
								// compare current folder & unfolded symlink name in current folder
								if(GetRealFileName(src) != GetRealFileName(src+"\\"+fd.cFileName))
								{
									if(!AddFile(src+"\\"+fd.cFileName, dst+"\\"+fd.cFileName, fd, flags, Level+1))
									{
										FindClose(hf);
										return FALSE;
									}
								}
							}
							else
							{
								if(!AddFile(src+"\\"+fd.cFileName, dst+"\\"+fd.cFileName, fd, flags, Level+1))
								{
									FindClose(hf);
									return FALSE;
								}
							}
						}
					}
					if(!FindNextFile(hf, &fd)) break;
				}
				FindClose(hf);
				if(descidx!=-1)
					if(!AddRemembered(Remember)) return FALSE;
			}
		}
		else if(Streams)
		{
			WIN32_STREAM_ID sid;
			ZeroMemory(&sid, sizeof(sid));
			wchar_t strn[1024];

			HANDLE hf = CreateFile(src.ptr(), GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, 0, NULL);
			if(hf!=INVALID_HANDLE_VALUE)
			{
				DWORD hsz=(DWORD)((LPBYTE)&sid.cStreamName-(LPBYTE)&sid);
				LPVOID ctx=NULL;
				while (1)
				{
					DWORD cb1, cb2;
					if(!BackupRead(hf, (LPBYTE)&sid, hsz, &cb1, FALSE, FALSE, &ctx)
						|| !cb1) break;
					strn[0]=0;
					if(sid.dwStreamNameSize)
						if(!BackupRead(hf, (LPBYTE)strn, sid.dwStreamNameSize, &cb2, FALSE, FALSE, &ctx)
							|| !cb2) break;
					if((sid.dwStreamId==BACKUP_DATA ||
						sid.dwStreamId==BACKUP_EA_DATA ||
						sid.dwStreamId==BACKUP_ALTERNATE_DATA)
						&& sid.dwStreamNameSize)
					{
						strn[sid.dwStreamNameSize/2]=0;
						if(!AddFile(src+strn, dst+strn, attr,
							sid.Size.QuadPart, creationTime, lastAccessTime, lastWriteTime, flags | AF_STREAM, Level+1))
						{
							BackupRead(NULL, NULL, 0, NULL, TRUE, FALSE, &ctx);
							CloseHandle(hf);
							return FALSE;
						}
					}
					BackupSeek(hf, sid.Size.LowPart, sid.Size.HighPart,
						&cb1, &cb2, &ctx);
				}
				BackupRead(NULL, NULL, 0, NULL, TRUE, FALSE, &ctx);
				CloseHandle(hf);
			}
		}
	}

fin:
	if(attr & FILE_ATTRIBUTE_DIRECTORY) {
		FileStruct info;
		memset(&info, 0, sizeof(info));
		info.Flags|=FLG_DIR_POST | info.Flags & (FLG_COPIED | FLG_DELETED);
		info.Level = Level;
		info.PanelIndex = -1;
		Files.push_back(info);

		SrcNames.AddRel(FileName::levelMinus, ExtractFileName(src));
		DstNames.AddRel(FileName::levelMinus, ExtractFileName(dst));
	}

	return TRUE;
}

void Engine::SetOverwriteMode(int Start)
{
	FileNameStoreEnum Enum(&DstNames);
	for (size_t i=Start; i<Enum.Count(); i++)
	{
		String fn=Enum.GetByNum(i);
		FileStruct &info=Files[i];
		if(!SkipNewer && info.Flags & FLG_SKIPNEWER)
		{
			info.Flags &= ~(FLG_SKIPNEWER | FLG_SKIPPED);
			TotalBytes += info.Size;
			TotalN++;
		}
		// bug #47 fixed by axxie
		if (!(info.Flags & FLG_SKIPPED) && !(info.Flags & FLG_DIR_PRE) && !(info.Flags & FLG_DIR_POST) && FileExists(fn)) {
			if(SkipNewer && Newer(fn, info.lastWriteTime)) {
				TotalBytes -= info.Size;
				TotalN--;
				info.Flags |= FLG_SKIPPED | FLG_SKIPNEWER;
			} else {
				switch (OverwriteMode)	{
					case OM_SKIP: {
						TotalBytes -= info.Size;
						TotalN--;
						info.Flags |= FLG_SKIPPED;
						break;
					}
					case OM_RENAME:	{
						int j=0;
						while (ExistsN(fn, j)) j++;
						info.RenameNum = j;
						break;
					}
				}
			}
		}
	}
}

int Engine::CheckOverwrite(int fnum, const String& Src, const String& Dst, String& ren)
{
	FarDialog& dlg = plugin->Dialogs()["OverwriteDialog"];
	dlg.ResetControls();

	dlg["Label2"]("Text")=Dst;
	String ssz, dsz, stime, dtime, buf;

	WIN32_FIND_DATA fd, fs;
	FindClose(FindFirstFile(Dst.ptr(), &fd));
	dsz=FormatNum(MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh));
	dtime=FormatTime(fd.ftLastWriteTime);
	dlg["Label4"]("Text")=Format(L"%14s %s	%s", dsz.ptr(), LOC("OverwriteDialog.Bytes").ptr(), dtime.ptr());

	FindClose(FindFirstFile(Src.ptr(), &fs));
	ssz=FormatNum(MAKEINT64(fs.nFileSizeLow, fs.nFileSizeHigh));
	stime=FormatTime(fs.ftLastWriteTime);
	dlg["Label3"]("Text")=Format(L"%14s %s	%s", ssz.ptr(), LOC("OverwriteDialog.Bytes").ptr(), stime.ptr());

	dlg["Source"]("Text")			 = Format(L"%-16s", LOC("OverwriteDialog.Source").ptr());
	dlg["Destination"]("Text")	= Format(L"%-16s", LOC("OverwriteDialog.Destination").ptr());

	dlg["AcceptForAll"]("Selected")=0;
	dlg["SkipIfNewer"]("Selected")=SkipNewer;
	dlg["SkippedToTemp"]("Selected")=SkippedToTemp;
	int OldSkipNewer=SkipNewer;

	int res=OM_PROMPT, ores=-1;
rep:
	switch (dlg.Execute())
	{
		case 0: res = OM_OVERWRITE;
			break;
		case 1: res = OM_SKIP;
			break;
		case 2: res = OM_APPEND;
			break;
		case 3: res = OM_RENAME;
			break;
		case -1:
			return OM_CANCEL;
	}

	int AcceptForAll=dlg["AcceptForAll"]("Selected");
	SkipNewer=dlg["SkipIfNewer"]("Selected");
	SkippedToTemp=dlg["SkippedToTemp"]("Selected");

	if(SkipNewer && Newer(fd.ftLastWriteTime, fs.ftLastWriteTime))
	{
		ores = res;
		res = OM_SKIP;
	}

	if(res == OM_RENAME && !AcceptForAll)
	{
		FarDialog& dlg = plugin->Dialogs()["RenameDialog"];
		dlg.ResetControls();

		dlg["Edit"]("Text")=ExtractFileName(Dst);
rep1:
		if(dlg.Execute()==0)
		{
			ren=ExtractFilePath(Dst)+"\\"+dlg["Edit"]("Text");
			if(ExistsN(ren, 0)) goto rep1;
		}
		else goto rep;
	}

	if(AcceptForAll || SkipNewer != OldSkipNewer)
	{
		if(AcceptForAll)
			if(ores==-1) OverwriteMode = res;
			else OverwriteMode = ores;
		if(fnum != -1)
		{
			//FarProgress progress;
			//progress.ShowMessage(LOC("Status.ScanningDest"));

			SetOverwriteMode(fnum+1);
			if(res == OM_RENAME)
			{
				int j = 0;
				while (ExistsN(Dst, j)) j++;
				ren=DupName(Dst, j);
			}
		}
	}

	return res;
}

void Engine::RememberFile(const String& Src, const String& Dst,
	WIN32_FIND_DATA &fd, int Flags, int Level,
	RememberStruct &Remember)
{
	Remember.Src=Src;
	Remember.Dst=Dst;
	Remember.Attr=fd.dwFileAttributes;
	Remember.Size=MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh);
	Remember.creationTime = fd.ftCreationTime;
	Remember.lastAccessTime = fd.ftLastAccessTime;
	Remember.lastWriteTime = fd.ftLastWriteTime;
	Remember.Flags=Flags;
	Remember.Level=Level;
}

int Engine::AddRemembered(RememberStruct &Remember)
{
	return AddFile(Remember.Src, Remember.Dst, Remember.Attr, Remember.Size, Remember.creationTime, Remember.lastAccessTime, Remember.lastWriteTime, Remember.Flags, Remember.Level);
}

// bugfixed by slst: bug #32
// Returns TRUE if there is enough space on target disk
BOOL Engine::CheckFreeDiskSpace(const int64_t TotalBytesToProcess, const int MoveMode,
	const String& srcpathstr, const String& dstpathstr)
{
	//if(ReplaceMode == OM_OVERWRITE) return TRUE;

	String srcroot = GetFileRoot(srcpathstr);
	String dstroot = GetFileRoot(dstpathstr);

	if((MoveMode) && (srcroot == dstroot)) return TRUE;

	BOOL result = FALSE;

	ULARGE_INTEGER FreeBytesAvailable;
	ULARGE_INTEGER TotalNumberOfBytes;
	ULARGE_INTEGER TotalNumberOfFreeBytes;

	if(GetDiskFreeSpaceEx(dstroot.ptr(), &FreeBytesAvailable,
												&TotalNumberOfBytes, &TotalNumberOfFreeBytes))
	{
		if(FreeBytesAvailable.QuadPart < (ULONGLONG)TotalBytesToProcess)
		{
			WaitForSingleObject(UiFree, INFINITE);

			FarDialog& dlg = plugin->Dialogs()["FreeSpaceErrorDialog"];
			dlg.ResetControls();

			dlg("Title") = LOC("FreeSpaceErrorDialog.Title");
			String disk_str = dstroot;
			if(disk_str.len() >= 2)
				if(disk_str[1] == ':') disk_str = disk_str.left(2);
			dlg["Label1"]("Text") = LOC("FreeSpaceErrorDialog.NotEnoughSpace") + " " + disk_str;
			dlg["Label2"]("Text") = Format(L"%-20s%12s", LOC("FreeSpaceErrorDialog.AvailableSpace").ptr(),
																			FormatValue(FreeBytesAvailable.QuadPart).ptr());
			dlg["Label3"]("Text") = Format(L"%-20s%12s", LOC("FreeSpaceErrorDialog.RequiredSpace").ptr(),
																			FormatValue(TotalBytesToProcess).ptr());
			dlg["Label4"]("Text") = LOC("FreeSpaceErrorDialog.AbortPrompt");

			int dlgres = dlg.Execute();

			HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
			FlushConsoleInputBuffer(h);
			SetEvent(UiFree);

			if(dlgres == RES_YES)
				result = FALSE;
			else
				result = TRUE;
		}
		else
			result = TRUE;
	}
	else
	result = TRUE;

	return result;
}

#define check(a, b) a ? b : NULL

void Engine::setFileTime(HANDLE h, FILETIME *creationTime, FILETIME *lastAccessTime, FILETIME *lastWriteTime)
{
	setFileTime2(h, check(copyCreationTime, creationTime), check(copyLastAccessTime, lastAccessTime), check(copyLastWriteTime, lastWriteTime));
}

void Engine::setFileSizeAndTime(const String& fn, int64_t size, FILETIME *creationTime, FILETIME *lastAccessTime, FILETIME *lastWriteTime)
{
	setFileSizeAndTime2(fn, size, check(copyCreationTime, creationTime), check(copyLastAccessTime, lastAccessTime), check(copyLastWriteTime, lastWriteTime));
}

int Engine::EngineError(const String& s, const String& fn, int code, int& flg, const String& title, const String& type_id)
{
	int *ix = NULL;
	if (flg & eeAutoSkipAll) {
		ix = &errTypes[type_id];
		if (*ix & errfSkipAll) {
			if (flg & eeShowKeepFiles && *ix & errfKeepFiles) {
								flg |= eerKeepFiles;
						}
						return RES_SKIP;
				}
		}

    FarDialog& dlg = plugin->Dialogs()["CopyError"];
    dlg.ResetControls();

    if (!title.empty()) {
        dlg("Title") = title;
    }

    if (flg & eeShowReopen) {
        dlg["Reopen"]("Visible") = 1;
    }
    if (flg & eeShowKeepFiles) {
        dlg["KeepFiles"]("Visible") = 1;
    }
    if (flg & eeYesNo) {
        dlg["YesNo"]("Visible") = 1;
    }
    if (flg & eeRetrySkipAbort) {
    dlg["RetrySkipAbort"]("Visible") = 1;
        if (flg & eeAutoSkipAll) {
            dlg["SkipAll"]("Visible") = 1;
        }
    }
    dlg["Label1"]("Text") = s;
    dlg["Label1"]("Visible") = 1;
    if (flg & eeOneLine) {
        dlg["Sep1"]("Visible")=0;
    } else {
        dlg["Label2"]("Text") = FormatWidthNoExt(fn, msgw());
        dlg["Label2"]("Visible") = 1;
        StringVector list;
        list.loadFromString(SplitWidth(GetErrText(code), msgw()), '\n');
        for (size_t i=0; i<list.Count(); i++) {
            if (i<=7) {
                String name = String("Label")+String(i+3);
                dlg[name]("Visible") = 1;
                dlg[name]("Text") = list[i];
            }
        }
    }

    if (!(flg & eeShowReopen) && !(flg & eeShowKeepFiles)) {
        dlg["Sep1"]("Visible")=0;
    }

    int res=dlg.Execute();

    if ((bool)dlg["Reopen"]("Selected")) flg |= eerReopen;
    if ((bool)dlg["KeepFiles"]("Selected")) flg |= eerKeepFiles;

    if (flg & eeYesNo)  {
        if (res == 0) return RES_YES;
        if (res == -1) return RES_NO;
    } else if (flg & eeRetrySkipAbort) {
        if (res == 0) return RES_RETRY;
        if (res == 1) return RES_SKIP;
        if (res == -1) return RES_ABORT;
        if (res == 2)  {
            *ix |= errfSkipAll;
            if (flg & eerKeepFiles) {
        *ix |= errfKeepFiles;
      }
      return RES_SKIP;
    }
    }
    return -1;
}
