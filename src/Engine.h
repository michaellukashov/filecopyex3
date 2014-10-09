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

#include <vector>

#include "CopyProgress.h"
#include "Framework/FileNameStoreEnum.h"

struct FileStruct
{
  int64_t Size, Read, Written, ResumePos;
  FILETIME creationTime, lastAccessTime, lastWriteTime;
  intptr_t RenameNum;
  intptr_t OverMode;
  intptr_t Level, PanelIndex, SectorSize;
  DWORD Attr;
  DWORD Flags;
};

struct BuffStruct
{
  size_t NextPos, WritePos;
  intptr_t FileNumber;
  bool EndFlag;
};

class TBuffInfo
{
public:
  int64_t OrgSize;
  size_t BuffSize;
  uint8_t * Buffer;
  BuffStruct * BuffInf;
  HANDLE OutFile;
  intptr_t OutNum;
  String SrcName, DstName;
};

struct RememberStruct
{
  int64_t Size;
  FILETIME creationTime, lastAccessTime, lastWriteTime;
  intptr_t Level;
  DWORD Attr, Flags;
  String Src, Dst;
};

PluginPanelItem * GetPanelItem(HANDLE hPlugin, FILE_CONTROL_COMMANDS Command, intptr_t Param1);

struct TPanelItem
{
public:
  TPanelItem(size_t idx, bool active = true, bool selected = false);
  ~TPanelItem()
  {
    delete[] ppi;
  }
  PluginPanelItem * operator->() const { return ppi; }
protected:
  PluginPanelItem * ppi;
};

class Engine
{
public:
  Engine();

  enum MResult { MRES_NONE, MRES_OK, MRES_STDCOPY, MRES_STDCOPY_RET };
  MResult Main(bool move, bool curonly);

private:
  FileNameStore SrcNames, DstNames;
  std::vector<FileStruct> Files;
  FileNameStoreEnum FlushSrc, FlushDst;
  bool Parallel, SkipNewer, SkippedToTemp;
  int Streams, Rights,
      CompressMode, EncryptMode;
  intptr_t OverwriteMode;
  size_t BufSize;
  int64_t ReadSpeedLimit, WriteSpeedLimit;
  int ReadAlign;
  bool _CopyDescs, _ClearROFromCD, _DescsInDirs, _ConfirmBreak,
      _HideDescs, _UpdateRODescs, _InverseBars;
  int _PreallocMin, _UnbuffMin;
  bool copyCreationTime, copyLastAccessTime, copyLastWriteTime;
  bool Aborted, KeepFiles;
  intptr_t volatile LastFile, FileCount, CopyCount;
  int64_t volatile ReadCb, WriteCb, ReadTime, WriteTime, TotalBytes,
          ReadN, WriteN, TotalN, FirstWrite, StartTime;
//  int64_t _LastCheckEscape, _CheckEscapeInterval;

  TBuffInfo * wbuffInfo, * buffInfo;
  HANDLE BGThread, FlushEnd, UiFree;

  CopyProgress CopyProgressBox;
  FarProgress ScanFoldersProgressBox;
  size_t SectorSize;
  bool Move;

  void Copy();
  bool InitBuf(TBuffInfo * buffInfo);
  void UninitBuf(TBuffInfo * buffInfo);
  void SwapBufs(TBuffInfo * src, TBuffInfo * dst);
  bool CheckEscape(bool ShowKeepFilesCheckBox = true);
  bool AskAbort(bool ShowKeepFilesCheckBox = true);
  bool FlushBuff(TBuffInfo * buffInfo);
  void CheckDstFileExists(TBuffInfo * buffInfo, intptr_t fnum, FileStruct & info,
    const String & SrcName, const bool TryToOpenDstFile,
    String & DstName);
  void BGFlush();
  bool WaitForFlushEnd();
  friend uint32_t __stdcall FlushThread(void * p);
  void FinalizeBuf(TBuffInfo * buffInfo);
  void ProcessDesc(intptr_t fnum);
  void ShowReadName(const String &);
  void ShowWriteName(const String &);
  void ShowProgress(int64_t, int64_t, int64_t, int64_t, int64_t,
                    int64_t, int64_t, int64_t);
  int CheckOverwrite(intptr_t fnum, String & ren);
  static void Delay(int64_t, int64_t, volatile int64_t &, int64_t);

  intptr_t CheckOverwrite(intptr_t, const String &, const String &, String &);
  intptr_t CheckOverwrite2(intptr_t, const String &, const String &, String &);
  void SetOverwriteMode(intptr_t);
  bool AddFile(const String & _src, const String & _dst, DWORD Attr, int64_t Size, const FILETIME & creationTime, const FILETIME & lastAccessTime, const FILETIME & lastWriteTime, DWORD Flags, intptr_t Level, intptr_t PanelIndex = -1);
  bool AddFile(const String & Src, const String & Dst, WIN32_FIND_DATA & fd, DWORD Flags, intptr_t Level, intptr_t PanelIndex = -1);
  void AddTopLevelDir(const String & dir, const String & dstmask, DWORD Flags, FileName::Direction d);
  static void RememberFile(const String & Src, const String & Dst, WIN32_FIND_DATA & fd, DWORD Flags, intptr_t Level, RememberStruct &);
  bool AddRemembered(RememberStruct &);
  bool DirStart(const String & dir, const String & dst);
  bool DirEnd(const String & dir, const String & dst);
  static String FindDescFile(const String & dir, intptr_t * idx = nullptr);
  static String FindDescFile(const String & dir, WIN32_FIND_DATA & fd, intptr_t * idx = nullptr);

  void SetFileTime(HANDLE h, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime);
  void SetFileSizeAndTime(const String & fn, int64_t size, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime);

  static void FarToWin32FindData(
    const TPanelItem & tpi,
    OUT WIN32_FIND_DATA & wfd
  );
  String CurPathDesc;
  DWORD CurPathFlags, CurPathAddFlags;
  WIN32_FIND_DATA DescFindData;

  std::map<String, int> errTypes;
  intptr_t EngineError(const String & s, const String & fn, int code, uint32_t & flg,
                  const String & title = L"", const String & type_id = L"");
  void FWError2(const String & Msg);

  bool CheckFreeDiskSpace(int64_t TotalBytesToProcess, bool MoveMode,
                          const String & srcpathstr, const String & dstpathstr);
};
