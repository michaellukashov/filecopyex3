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

#include <process.h>
#if defined(_MSC_VER)
#include <memory>
#else
#include <c++/bits/unique_ptr.h>
#endif
#include "Framework/DescList.h"
#include "Framework/ObjString.h"
#include "Framework/FileUtils.h"
#include "Framework/StrUtils.h"
#include "Common.h"
#include "Engine.h"
#include "guid.hpp"
#include "FarPlugin.h"
#include "EngineTools.h"
#include "Tools.h"
#include "ui.h"

static const size_t AllocAlign = 64 * 1024;
static const size_t ReadBlock = 1024 * 1024;
static const size_t WriteBlock = 1024 * 1024;

static PluginPanelItem * GetPanelItem(HANDLE hPlugin, FILE_CONTROL_COMMANDS Command, intptr_t Param1)
{
  size_t Size = Info.PanelControl(hPlugin, Command, Param1, 0);
  PluginPanelItem * item = reinterpret_cast<PluginPanelItem *>(new char[Size]);

  FarGetPluginPanelItem gpi = { sizeof(FarGetPluginPanelItem), Size, item };
  Info.PanelControl(hPlugin, Command, Param1, &gpi);
  return item;
}

static intptr_t warn(const String & s)
{
  return ShowMessage(LOC(L"CopyDialog.Warning"), s + L"\n" + LOC(L"CopyDialog.WarnPrompt"), FMSG_MB_YESNO) == 0;
}

static void ShowErrorMessage(const String & s)
{
  //ShowMessage(LOC(L"CopyDialog.Error"), s, FMSG_WARNING | FMSG_MB_OK);

  String msgbuf = LOC(L"CopyDialog.Error") + L"\n" + s + L"\n";

  Info.Message(&MainGuid, &UnkGuid, FMSG_WARNING | FMSG_MB_OK | FMSG_ALLINONE, nullptr, (const wchar_t * const *)msgbuf.ptr(), 0, 0);
}

static int64_t GetPhysMemorySize()
{
  MEMORYSTATUS ms;
  ::GlobalMemoryStatus(&ms);
  return ms.dwTotalPhys;
}

Engine::Engine() :
  FlushSrc(&SrcNames),
  FlushDst(&DstNames),
  BGThread(nullptr),
  FlushEnd(nullptr),
  UiFree(nullptr)
{
  _CopyDescs = false;
  _ClearROFromCD = false;
  _DescsInDirs = false;
  _ConfirmBreak = false;
  _HideDescs = false;
  _UpdateRODescs = false;
  _InverseBars = false;
  _PreallocMin = 0;
  _UnbuffMin = 0;
  copyCreationTime = false;
  copyLastAccessTime = false;
  copyLastWriteTime = false;
  Aborted = false;
  KeepFiles = false;
  LastFile = 0;
  FileCount = 0;
  CopyCount = 0;
//  _LastCheckEscape = 0;
//  _CheckEscapeInterval = TicksPerSec() / 2;
  wbuffInfo = nullptr;
  buffInfo = nullptr;

  Parallel = SkipNewer = SkippedToTemp = false;
  Streams = Rights = 0;
  Move = false;
  CompressMode = EncryptMode = ATTR_INHERIT;
  OverwriteMode = OM_PROMPT;
  BufSize = 0;

  SYSTEM_INFO si;
  ::GetSystemInfo(&si);
  ReadAlign = si.dwPageSize;

  SectorSize = 0;
  CurPathFlags = 0;
  CurPathAddFlags = 0;
  ReadSpeedLimit = 0;
  WriteSpeedLimit = 0;
  ReadCb = 0;
  WriteCb = 0;
  ReadTime = 0;
  WriteTime = 0;
  TotalBytes = 0;
  ReadN = 0;
  WriteN = 0;
  TotalN = 0;
  FirstWrite = 0;
  StartTime = 0;
  ::ZeroMemory(&DescFindData, sizeof(DescFindData));
}

bool Engine::InitBuf(TBuffInfo * ABuffInfo)
{
  ABuffInfo->OutFile = nullptr;
  ABuffInfo->OutNum = -1;

  ABuffInfo->BuffSize = BufSize;
  if (Parallel)
    ABuffInfo->BuffSize /= 2;
  ABuffInfo->BuffSize = (ABuffInfo->BuffSize / AllocAlign + 1) * AllocAlign;

  ABuffInfo->Buffer = static_cast<uint8_t *>(Alloc(ABuffInfo->BuffSize));
  if (!ABuffInfo->Buffer)
  {
    Error(LOC(L"Error.MemAlloc"), ::GetLastError());
    return FALSE;
  }
  ABuffInfo->BuffInf = static_cast<BuffStruct *>(Alloc(SrcNames.Count() * sizeof(BuffStruct)));
  if (!ABuffInfo->BuffInf)
  {
    Free(ABuffInfo->Buffer);
    Error(LOC(L"Error.MemAlloc"), ::GetLastError());
    return false;
  }

  return true;
}

void Engine::UninitBuf(TBuffInfo * buffInfo)
{
  Free(buffInfo->Buffer);
  Free(buffInfo->BuffInf);
}

void Engine::SwapBufs(TBuffInfo * src, TBuffInfo * dst)
{
  memcpy(dst->Buffer, src->Buffer, src->BuffSize);
  memcpy(dst->BuffInf, src->BuffInf, SrcNames.Count() * sizeof(BuffStruct));
}

bool Engine::AskAbort(bool ShowKeepFilesCheckBox)
{
  if (_ConfirmBreak)
  {
    ::WaitForSingleObject(UiFree, INFINITE);

    uint32_t flg = eeYesNo | eeOneLine;
    if (ShowKeepFilesCheckBox)
      flg |= eeShowKeepFiles;

    intptr_t res = EngineError(LOC(L"CopyError.StopPrompt"), L"", 0, flg, LOC(L"CopyError.StopTitle"));

    HANDLE h = ::GetStdHandle(STD_INPUT_HANDLE);
    ::FlushConsoleInputBuffer(h);
    ::SetEvent(UiFree);

    Aborted = (res == 0);
    if (Aborted)
      KeepFiles = (flg & eerKeepFiles) != 0;
    return Aborted;
  }
  else
  {
    KeepFiles = false;
    return true;
  }
}

bool Engine::CheckEscape(bool ShowKeepFilesCheckBox)
{
  //int64_t tm = GetTime();
  //if (tm - _LastCheckEscape < _CheckEscapeInterval)
  //{
  //  return FALSE;
  //}
  //_LastCheckEscape = tm;
  if (::WaitForSingleObject(UiFree, 0) == WAIT_TIMEOUT)
    return false;

  bool escape = false;
  HANDLE h = ::GetStdHandle(STD_INPUT_HANDLE);
  while (1)
  {
    INPUT_RECORD rec;
    DWORD rc;
    ::PeekConsoleInput(h, &rec, 1, &rc);
    if (!rc)
      break;
    ::ReadConsoleInput(h, &rec, 1, &rc);
    if (rec.EventType == KEY_EVENT)
      if (rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE &&
          !(rec.Event.KeyEvent.dwControlKeyState &
               (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED | LEFT_ALT_PRESSED |
                RIGHT_ALT_PRESSED | SHIFT_PRESSED)) &&
          rec.Event.KeyEvent.bKeyDown)
      {
        ::FlushConsoleInputBuffer(h);
        escape = true;
      }
  }

  ::SetEvent(UiFree);

  if (escape && !AskAbort(ShowKeepFilesCheckBox))
    escape = false;

  return escape;
}

void Engine::FinalizeBuf(TBuffInfo * ABuffInfo)
{
  HANDLE Handle = ABuffInfo->OutFile;
  intptr_t fnum = ABuffInfo->OutNum;
  String & DstName = ABuffInfo->DstName;
  String & SrcName = ABuffInfo->SrcName;
  FileStruct & info = Files[fnum];

  if (!(info.Flags & FLG_SKIPPED))
  {
    if (info.Flags & FLG_BUFFERED)
    {
      this->SetFileTime(Handle, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
    }
  }

  FClose(Handle);

  if (!(info.Flags & FLG_SKIPPED))
  {
    info.Flags |= FLG_COPIED;
    LastFile = fnum;

    if (!(info.Flags & FLG_BUFFERED))
    {
      if (info.OverMode == OM_APPEND)
      {
        SetFileSizeAndTime(DstName, ABuffInfo->OrgSize + info.Size, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
      }
      else
      {
        SetFileSizeAndTime(DstName, info.Size, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
      }
    }

    if (Rights)
      FCopyACL(SrcName, DstName);
    FEncrypt(DstName, EncryptMode);
    ::SetFileAttributes(DstName.ptr(), info.Attr);

    if (Move)
    {
      bool DelRetry = true;
      while (DelRetry)
      {
        DelRetry = false;
        if (FileExists(SrcName) && !FDelete(SrcName))
        {
          ::WaitForSingleObject(UiFree, INFINITE);
          uint32_t flg = eeRetrySkipAbort | eeAutoSkipAll;
          intptr_t res = EngineError(LOC(L"Error.FileDelete"), SrcName, ::GetLastError(), flg, L"", L"Error.FileDelete");
          ::SetEvent(UiFree);
          DelRetry = (res == RES_RETRY);
          if (res == RES_ABORT)
          {
            Aborted = true;
          }
        }
        else
        {
          info.Flags |= FLG_DELETED;
          DelRetry = false;
        }
      }
    }

    WriteN++;

    size_t dnum = fnum + 1;
    while (dnum < SrcNames.Count() && Files[dnum].Flags & (FLG_DIR_POST | FLG_DIR_PRE | FLG_DESCFILE))
    {
      if (!(Files[dnum].Flags & FLG_COPIED))
      {
        if (Files[dnum].Flags & FLG_DESCFILE)
        {
          ProcessDesc(dnum);
        }
        else if (Files[dnum].Flags & FLG_DIR_POST && !(Files[dnum].Flags & FLG_DIR_NOREMOVE) && Move)
        {
          if (RmDir(FlushSrc.GetByNum(dnum)))
          {
            Files[dnum].Flags |= FLG_COPIED | FLG_DELETED;
          }
        }
      }
      dnum++;
    }
  }
  else
  {
    if (!(info.Flags & FLG_DECSIZE))
    {
      WriteCb -= info.Written;
      ReadCb -= info.Read;
      TotalBytes -= info.Size;
      TotalN--;
      info.Flags |= FLG_DECSIZE;
    }

    if (Handle != INVALID_HANDLE_VALUE)
    {
      if (info.OverMode == OM_APPEND || info.OverMode == OM_RESUME)
      {
        if (KeepFiles || (info.Flags & FLG_KEEPFILE))
        {
          SetFileSizeAndTime(DstName, ABuffInfo->OrgSize + info.Written, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
        }
        else
        {
          SetFileSizeAndTime(DstName, ABuffInfo->OrgSize, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
        }
      }
      else
      {
        if (KeepFiles || (info.Flags & FLG_KEEPFILE))
        {
          SetFileSizeAndTime(DstName, info.Written, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
        }
        else
        {
          FDelete(DstName);
        }
      }
      ::SetFileAttributes(DstName.ptr(), info.Attr);
    }
  }
}

void Engine::ProcessDesc(intptr_t fnum)
{
  String DstName = FlushDst.GetByNum(fnum);
  String SrcName = FlushSrc.GetByNum(fnum);
  //TODO: OM_RENAME can change DstName
  FileStruct & info = Files[fnum];

  String DstPath = AddEndSlash(ExtractFilePath(DstName));
  String df = FindDescFile(DstPath);
  if (!df.empty())
  {
    DstName = DstPath + df;
  }

  DescList SrcList, DstList;
  SrcList.LoadFromFile(SrcName);
  DWORD attr = ::GetFileAttributes(DstName.ptr());
  if (!_UpdateRODescs && attr != INVALID_FILE_ATTRIBUTES &&
      (attr & FILE_ATTRIBUTE_READONLY))
    return;
  DstList.LoadFromFile(DstName);

  bool inverse = (info.Flags & FLG_DESC_INVERSE) != 0;
  if (!inverse)
  {
    if (Move)
      SrcList.SetAllSaveFlags(0);
  }
  else
    SrcList.SetAllMergeFlags(0);

  intptr_t j = fnum - 1;
  while (j >= 0 && Files[j].Level >= Files[fnum].Level)
  {
    if (Files[j].Level == Files[fnum].Level)
    {
      String sn = SrcNames.GetNameByNum(j);
      String dn = DstNames.GetNameByNum(j);
      int Same = 0;

      if (sn.cmp(dn))
      {
        SrcList.Rename(sn, dn, 1);
        sn = dn;
        if (!SrcName.icmp(DstName))
          Same = 1;
      }

      if (!(Files[j].Flags & FLG_DIR_POST) &&
          ((Files[j].Flags & FLG_COPIED) != 0) == inverse)
      {
        SrcList.SetMergeFlag(sn, inverse);
      }
      if (Move && !(Files[j].Flags & FLG_DIR_PRE))
      {
        if (Same)
          SrcList.SetSaveFlag(sn, 1);
        else if (((Files[j].Flags & FLG_DELETED) != 0) == inverse)
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
      ::SetFileAttributes(DstName.ptr(),
                        ::GetFileAttributes(DstName.ptr()) | FILE_ATTRIBUTE_HIDDEN);
    if (Move)
    {
      DWORD attr = ::GetFileAttributes(SrcName.ptr());
      if (!_UpdateRODescs && attr != INVALID_FILE_ATTRIBUTES &&
          (attr & FILE_ATTRIBUTE_READONLY))
        return;
      if (!SrcList.SaveToFile(SrcName))
        Error2(LOC(L"Error.WriteDesc"), SrcName, ::GetLastError());
    }
  }
  else
  {
    Error2(LOC(L"Error.WriteDesc"), DstName, ::GetLastError());
    info.Flags |= FLG_SKIPPED | FLG_ERROR;
  }
}

void Engine::CheckDstFileExists(TBuffInfo * buffInfo, intptr_t fnum, FileStruct & info,
  const String & SrcName,
  const bool TryToOpenDstFile,
  String & DstName)
{
  if (FileExists(DstName))
  {
    if (info.OverMode == OM_PROMPT)
      info.OverMode = OverwriteMode;
    if (info.OverMode == OM_RENAME && info.RenameNum)
      DstName = DupName(DstName, info.RenameNum);
    bool Repeat = true;
    while (Repeat)
    {
      Repeat = false;
      if (info.OverMode == OM_PROMPT)
      {
        String renn;
        info.OverMode = CheckOverwrite2(fnum, SrcName, DstName, renn);
        if (info.OverMode == OM_RENAME)
          DstName = renn;
      }
      else
        info.OverMode = OM_OVERWRITE;

      if (EncryptMode == ATTR_ON)
        info.Attr |= FILE_ATTRIBUTE_ENCRYPTED;
      else
        info.Attr &= ~FILE_ATTRIBUTE_ENCRYPTED;

      buffInfo->SrcName = SrcName;
      buffInfo->DstName = DstName;

      bool OpenRetry = true;
      while (OpenRetry)
      {
        OpenRetry = false;
        uint32_t oflg = info.Flags & FLG_BUFFERED ? OPEN_BUF : 0;
        switch (info.OverMode)
        {
          case OM_OVERWRITE:
          case OM_RENAME:
            if (TryToOpenDstFile)
            {
              buffInfo->OutFile = FOpen(DstName, OPEN_CREATE | oflg, info.Attr);
            }
            break;
          case OM_APPEND:
            info.Flags |= FLG_BUFFERED;
            if (TryToOpenDstFile)
            {
              oflg |= OPEN_BUF;
                buffInfo->OutFile = FOpen(DstName, OPEN_APPEND | oflg, 0);
            }
            break;
          case OM_RESUME:
            if (TryToOpenDstFile)
            {
              buffInfo->OutFile = FOpen(DstName, OPEN_WRITE | oflg, 0);
              if (FSeek(buffInfo->OutFile, info.ResumePos, FILE_BEGIN) == -1)
                FWError2(Format(L"FSeek to %d failed, code %d", info.ResumePos,
                               (int)::GetLastError()));
            }
            break;
          case OM_SKIP:
            info.Flags |= FLG_SKIPPED;
            break;
          case OM_PROMPT:
          case OM_CANCEL:
            if (AskAbort())
              info.Flags |= FLG_SKIPPED;
            else
              Repeat = true;
            break;
        }
      }

      ShowReadName(SrcName);
      ShowWriteName(DstName);

      if (!(info.Flags & FLG_SKIPPED))
      {
        if (buffInfo->OutFile)
        {
          FCompress(buffInfo->OutFile, CompressMode);
          //Encrypt(bi->OutFile, EncryptMode);
          buffInfo->OrgSize = FileSize(buffInfo->OutFile);

          int64_t size = info.OverMode == OM_APPEND ? buffInfo->OrgSize + info.Size : info.Size;
          if (size >= (int64_t)_PreallocMin * 1024 &&
              FGetCompression(buffInfo->OutFile) == 0)
          {
            int64_t sp;
            if (!(info.Flags & FLG_BUFFERED))
            {
              sp = size / info.SectorSize;
              if (size % info.SectorSize)
                sp++;
              sp *= info.SectorSize;
            }
            else
              sp = size;
            int64_t bp = FTell(buffInfo->OutFile);
            FSeek(buffInfo->OutFile, sp, FILE_BEGIN);
            SetEndOfFile(buffInfo->OutFile);
            FSeek(buffInfo->OutFile, bp, FILE_BEGIN);
          }
        }
        else if (TryToOpenDstFile)
        {
          ::WaitForSingleObject(UiFree, INFINITE);
          uint32_t flg = eeRetrySkipAbort | eeAutoSkipAll;
          intptr_t res = EngineError(LOC(L"Error.OutputFileCreate"), DstName, ::GetLastError(),
                                flg, L"", L"Error.OutputFileCreate");
          ::SetEvent(UiFree);
          OpenRetry = (res == RES_RETRY);
          if (res == RES_ABORT)
            Aborted = true;
        }
      }
    }
  }
}

bool Engine::FlushBuff(TBuffInfo * ABuffInfo)
{
  size_t Pos = 0;
  size_t PosInStr = 0;

  while (Pos < ABuffInfo->BuffSize && ABuffInfo->BuffInf[PosInStr].FileNumber >= 0)
  {
    intptr_t fnum = ABuffInfo->BuffInf[PosInStr].FileNumber;
    String SrcName = FlushSrc.GetByNum(fnum);
    String DstName = FlushDst.GetByNum(fnum);
    FileStruct & info = Files[fnum];
    ABuffInfo->OutNum = fnum;

    if (Aborted)
      info.Flags |= FLG_SKIPPED;

    if (!ABuffInfo->OutFile && !(info.Flags & FLG_SKIPPED))
    {
      CheckDstFileExists(ABuffInfo, fnum, info, SrcName, true, DstName);
    }
    if (!(info.Flags & FLG_SKIPPED) && !ABuffInfo->OutFile)
      info.Flags |= FLG_SKIPPED | FLG_ERROR;

    bool Skipped = false;
    while(1)
    {
      if (!(info.Flags & FLG_SKIPPED))
      {
        while (Pos < ABuffInfo->BuffInf[PosInStr].WritePos)
        {
          if (CheckEscape() || Aborted)
          {
            info.Flags |= FLG_SKIPPED;
            Skipped = true;
            break;
          }

          size_t write_block_size = Min(ABuffInfo->BuffInf[PosInStr].WritePos - Pos, WriteBlock);
          size_t write_block_size1 = write_block_size;
          if (info.Flags & FLG_BUFFERED)
            write_block_size = Min(write_block_size, (size_t)(info.Size - info.Written));
          int64_t start_time = 0;
          size_t written = 0;
          bool Retry = true;
          while (Retry)
          {
            Retry = false;
            start_time = GetTime();
            written = FWrite(ABuffInfo->OutFile, ABuffInfo->Buffer + Pos, write_block_size);

            if (written < write_block_size)
            {
              ::WaitForSingleObject(UiFree, INFINITE);
              uint32_t flg = eeShowReopen | eeShowKeepFiles | eeRetrySkipAbort | eeAutoSkipAll;
              intptr_t res = EngineError(LOC(L"Error.Write"), DstName, ::GetLastError(), flg,
                                    L"", L"Error.Write");
              ::SetEvent(UiFree);
              if (res == RES_RETRY)
              {
                if (flg & eerReopen)
                {
                  int64_t Pos = info.Written;
                  if (info.OverMode == OM_RESUME)
                    Pos += info.ResumePos;
                  FClose(ABuffInfo->OutFile);
                  bool ReopenRetry = true;
                  while (ReopenRetry)
                  {
                    ReopenRetry = false;
                    uint32_t oflg = info.Flags & FLG_BUFFERED ? OPEN_BUF : 0;
                    ABuffInfo->OutFile = FOpen(DstName, OPEN_WRITE | oflg, 0);
                    if (!ABuffInfo->OutFile)
                    {
                      ::WaitForSingleObject(UiFree, INFINITE);
                      uint32_t flg = eeShowKeepFiles | eeRetrySkipAbort/* | eeAutoSkipAll*/;
                      intptr_t res = EngineError(LOC(L"Error.OutputFileCreate"),
                                            DstName, ::GetLastError(), flg, L"", L"Error.OutputFileCreate");
                      ::SetEvent(UiFree);
                      ReopenRetry = (res == RES_RETRY);
                      if (!ReopenRetry)
                      {
                        info.Flags |= FLG_SKIPPED | FLG_ERROR;
                        if (flg & eerKeepFiles)
                          info.Flags |= FLG_KEEPFILE;
                        ABuffInfo->OutFile = INVALID_HANDLE_VALUE;
                        if (res == RES_ABORT)
                          Aborted = true;
                        Skipped = true;
                        break;
                      }
                    }
                  }
                  if (Aborted || Skipped)
                    break;
                  if (FSeek(ABuffInfo->OutFile, Pos, FILE_BEGIN) == -1)
                    FWError2(Format(L"FSeek to %d failed, code %d", Pos,
                                   (int)::GetLastError()));
                }
                Retry = true;
                break;
              }
              else
              {
                info.Flags |= FLG_SKIPPED | FLG_ERROR;
                if (flg & eerKeepFiles)
                  info.Flags |= FLG_KEEPFILE;
                if (res == RES_ABORT)
                  Aborted = true;
                Skipped = true;
                break;
              }
            }
          }
          if (Aborted || Skipped)
            break;

          int64_t write_time = GetTime() - start_time;
          WriteTime += write_time;
          info.Written += written;
          WriteCb += written;
          Pos += written;
          if (!FirstWrite)
            FirstWrite = GetTime() - StartTime;

          Delay(write_time, written, WriteTime, WriteSpeedLimit);
          ShowReadName(SrcName);
          ShowWriteName(DstName);
          ShowProgress(ReadCb, WriteCb, TotalBytes, ReadTime, WriteTime, ReadN, WriteN, TotalN);

          if (write_block_size < write_block_size1)
            Pos = ABuffInfo->BuffInf[PosInStr].WritePos;
        }
        if (Aborted || Skipped)
          break;
      }
      break;
    }
    Pos = ABuffInfo->BuffInf[PosInStr].NextPos;

    if (ABuffInfo->BuffInf[PosInStr].EndFlag || info.Flags & FLG_SKIPPED)
    {
      FinalizeBuf(ABuffInfo);
      ABuffInfo->OutFile = nullptr;
      ABuffInfo->OutNum = -1;
      ABuffInfo->SrcName.Clear();
      ABuffInfo->DstName.Clear();
    }

    if (Aborted)
      break;
    PosInStr++;
  }

  if (Parallel)
    ::SetEvent(FlushEnd);
  return !Aborted;
}

uint32_t __stdcall FlushThread(void * p)
{
  Engine * eng = static_cast<Engine *>(p);
  return eng->FlushBuff(eng->wbuffInfo);
}

void Engine::BGFlush()
{
  BGThread = (HANDLE)_beginthreadex(nullptr, 0, FlushThread, this, 0, nullptr);
}

bool Engine::WaitForFlushEnd()
{
  while (::WaitForSingleObject(FlushEnd, 200) == WAIT_TIMEOUT)
  {
    if (CheckEscape())
      return false;
  }
  ::WaitForSingleObject(BGThread, INFINITE);
  ::CloseHandle(BGThread);
  BGThread = nullptr;
  return true;
}

struct CurDirInfo
{
  size_t SectorSize;
};

void Engine::Copy()
{
  LastFile = -1;
  Aborted = false;
  KeepFiles = false;
  ReadCb = WriteCb = ReadTime = WriteTime = ReadN = WriteN = FirstWrite = 0;
  StartTime = GetTime();

  TBuffInfo _bi, _wbi;
  buffInfo = &_bi;
  wbuffInfo = &_wbi;
  if (!InitBuf(buffInfo))
    return;
  if (Parallel)
  {
    if (!InitBuf(wbuffInfo))
    {
      UninitBuf(buffInfo);
      return;
    }
    FlushEnd = ::CreateEvent(nullptr, FALSE, TRUE, nullptr);
  }

  size_t BuffPos = 0;
  size_t FilesInBuff = 0;

  UiFree = ::CreateEvent(nullptr, FALSE, TRUE, nullptr);

  CopyProgressBox.InverseBars = _InverseBars;
  if (FileCount)
    CopyProgressBox.Start(Move);

  FileNameStoreEnum Src(&SrcNames);
  FileNameStoreEnum Dst(&DstNames);
  std::vector<CurDirInfo> CurDirStack;

  for (size_t Index = 0; Index < Src.Count(); Index++)
  {
    String SrcName = Src.GetNext();
    String DstName = Dst.GetNext();

    HANDLE InputFileHandle = INVALID_HANDLE_VALUE;
    FileStruct & info = Files[Index];
    if (info.Flags & FLG_SKIPPED ||
        info.Flags & FLG_COPIED ||
        info.Flags & FLG_DESCFILE)
      continue;

    bool Skipped = false;

    auto AbortOperation = [&]() -> void
    {
      info.Flags |= FLG_SKIPPED;
      FClose(InputFileHandle);
      Aborted = true;
    };

    auto SkipOperation = [&]() -> bool
    {
      size_t abp = BuffPos;
      if (abp % SectorSize)
        abp = (abp / SectorSize + 1) * SectorSize;
      buffInfo->BuffInf[FilesInBuff].WritePos = abp;
      if (BuffPos % ReadAlign)
        BuffPos = (BuffPos / ReadAlign + 1) * ReadAlign;
      buffInfo->BuffInf[FilesInBuff].NextPos = BuffPos;
      buffInfo->BuffInf[FilesInBuff].FileNumber = (intptr_t)Index;
      if (BuffPos == buffInfo->BuffSize)
      {
        buffInfo->BuffInf[FilesInBuff].EndFlag = false;
        if (!Parallel)
        {
          if (!FlushBuff(buffInfo))
          {
            AbortOperation();
            return false;
          }
        }
        else
        {
          if (!WaitForFlushEnd())
          {
            AbortOperation();
            return false;
          }
          SwapBufs(buffInfo, wbuffInfo);
          BGFlush();
        }
        BuffPos = 0;
        FilesInBuff = 0;
      }
      else
      {
        buffInfo->BuffInf[FilesInBuff].EndFlag = true;
        FilesInBuff++;
        return false;
      }
      return true;
    };

    if (info.Flags & FLG_DIR_PRE)
    {
      if (info.Flags & FLG_DIR_FORCE)
        ForceDirectories(AddEndSlash(DstName));
      else
      {
        ::CreateDirectory(GetLongFileName(DstName).ptr(), nullptr);
        ::SetFileAttributes(DstName.ptr(), info.Attr);
      }
      if (!(info.Flags & FLG_TOP_DIR))
      {
        HANDLE hd = ::CreateFile(DstName.ptr(), GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                               OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (hd != INVALID_HANDLE_VALUE)
        {
          FCompress(hd, CompressMode);
          if (!(info.Flags & FLG_DIR_FORCE))
          {
            this->SetFileTime(hd, &info.creationTime, &info.lastAccessTime, &info.lastWriteTime);
          }
          ::CloseHandle(hd);
        }
        if (EncryptMode != ATTR_INHERIT)
          FEncrypt(DstName, EncryptMode);
        if (Rights && !(info.Flags & FLG_DIR_FORCE))
          FCopyACL(SrcName, DstName);
      }
      info.Flags |= FLG_COPIED;

      CurDirInfo cdi;
      DWORD dattr = ::GetFileAttributes(DstName.ptr());
      if (!CurDirStack.size() || (dattr != INVALID_FILE_ATTRIBUTES && dattr & FILE_ATTRIBUTE_REPARSE_POINT))
      {
        cdi.SectorSize = GetSectorSize(DstName);
      }
      else
      {
        cdi.SectorSize = CurDirStack.back().SectorSize;
        CurDirStack.pop_back();
      }
      CurDirStack.push_back(cdi);
      SectorSize = cdi.SectorSize;
      continue;
    }

    if (info.Flags & FLG_DIR_POST)
    {
      if (CurDirStack.size())
      {
        CurDirStack.pop_back();
      }
      if (CurDirStack.size())
      {
        SectorSize = CurDirStack[CurDirStack.size() - 1].SectorSize;
      }
      continue;
    }
    info.SectorSize = SectorSize;

    bool OpenRetry = true;
    intptr_t res = RES_NO;
    while (OpenRetry)
    {
      OpenRetry = false;
      InputFileHandle = FOpen(SrcName, OPEN_READ, 0);
      ShowReadName(SrcName);

      if (!buffInfo->OutFile && !(info.Flags & FLG_SKIPPED))
      {
        CheckDstFileExists(buffInfo, Index, info, SrcName, false, DstName);
      }

      if (!InputFileHandle)
      {
        ::WaitForSingleObject(UiFree, INFINITE);
        uint32_t flg = eeRetrySkipAbort | eeAutoSkipAll;
        res = EngineError(LOC(L"Error.InputFileOpen"), SrcName, ::GetLastError(),
                              flg, L"", L"Error.InputFileOpen");
        ::SetEvent(UiFree);
        OpenRetry = (res == RES_RETRY);
        if (!OpenRetry)
        {
          info.Flags |= FLG_SKIPPED | FLG_ERROR;
          if (res == RES_ABORT)
          {
            AbortOperation();
            break;
          }
          else
            break;
        }
      }
    }
    if (Aborted || Skipped)
      break;

    if (OverwriteMode == OM_RESUME)
    {
      if (FSeek(InputFileHandle, info.ResumePos, FILE_BEGIN) == -1)
        FWError2(Format(L"FSeek to %d failed, code %d", info.ResumePos,
                       (int)::GetLastError()));
    }

    while (1)
    {
      if (info.Flags & FLG_SKIPPED)
        break;
      if (info.Size < _UnbuffMin * 1024)
        info.Flags |= FLG_BUFFERED;

      while (BuffPos < buffInfo->BuffSize)
      {
        if (info.Flags & FLG_SKIPPED)
          break;
        size_t read_block_size = Min(ReadBlock, buffInfo->BuffSize - BuffPos);
        int64_t start_time = 0;
        size_t read = 0;
        bool Retry = true;
        while (Retry)
        {
          Retry = false;
          start_time = GetTime();
          read = FRead(InputFileHandle, buffInfo->Buffer + BuffPos, read_block_size);

          if (read == -1)
          {
            ::WaitForSingleObject(UiFree, INFINITE);
            uint32_t flg = eeShowReopen | eeShowKeepFiles | eeRetrySkipAbort | eeAutoSkipAll;
            intptr_t res = EngineError(LOC(L"Error.Read"), SrcName, ::GetLastError(), flg,
                                  L"", L"Error.Read");
            ::SetEvent(UiFree);
            if (res == RES_RETRY)
            {
              if (flg & eerReopen)
              {
                int64_t Pos = info.Read;
                if (info.OverMode == OM_RESUME)
                  Pos += info.ResumePos;
                FClose(InputFileHandle);
                bool ReopenRetry = true;
                while (ReopenRetry)
                {
                  ReopenRetry = false;
                  InputFileHandle = FOpen(SrcName, OPEN_READ, 0);
                  if (!InputFileHandle)
                  {
                    ::WaitForSingleObject(UiFree, INFINITE);
                    uint32_t flg = eeShowKeepFiles | eeRetrySkipAbort/* | eeAutoSkipAll*/;
                    intptr_t res = EngineError(LOC(L"Error.InputFileOpen"), SrcName,
                                          ::GetLastError(), flg, L"", L"Error.InputFileOpen");
                    ::SetEvent(UiFree);
                    ReopenRetry = (res == RES_RETRY);
                    if (!ReopenRetry)
                    {
                      info.Flags |= FLG_SKIPPED | FLG_ERROR;
                      if (flg & eerKeepFiles)
                        info.Flags |= FLG_KEEPFILE;
                      if (res == RES_ABORT)
                      {
                        AbortOperation();
                        break;
                      }
                      else
                      {
                        SkipOperation();
                        Skipped = true;
                        break;
                      }
                    }
                  }
                }
                if (Aborted || Skipped)
                  break;
                if (FSeek(InputFileHandle, Pos, FILE_BEGIN) == -1)
                  FWError2(Format(L"FSeek to %d failed, code %d", Pos,
                                 (int)::GetLastError()));
              }
              Retry = true;
              break;
            }
            else
            {
              info.Flags |= FLG_SKIPPED | FLG_ERROR;
              if (flg & eerKeepFiles)
                info.Flags |= FLG_KEEPFILE;
              if (res == RES_ABORT)
              {
                AbortOperation();
                break;
              }
              else
              {
                SkipOperation();
                Skipped = true;
                break;
              }
            }
          }
        }
        if (Aborted || Skipped)
          break;

        int64_t read_time = GetTime() - start_time;
        ReadTime += read_time;
        info.Read += read;
        ReadCb += read;
        BuffPos += read;

        Delay(read_time, read, ReadTime, ReadSpeedLimit);
        ShowProgress(ReadCb, WriteCb, TotalBytes, ReadTime, WriteTime, ReadN, WriteN, CopyCount);
        ShowReadName(SrcName);

        if (CheckEscape())
        {
          AbortOperation();
          break;
        }
        if (read < read_block_size)
          break;
      }
      if (Aborted || Skipped)
        break;

      if (!SkipOperation())
        break;
    }
    if (Aborted || Skipped)
      break;

    FClose(InputFileHandle);
    ReadN++;
    continue;

    AbortOperation();
    break;
  }

  if (Parallel)
  {
    while (!WaitForFlushEnd());
    buffInfo->OutFile = wbuffInfo->OutFile;
    buffInfo->OutNum = wbuffInfo->OutNum;
    buffInfo->SrcName = wbuffInfo->SrcName;
    buffInfo->DstName = wbuffInfo->DstName;
  }
  buffInfo->BuffInf[FilesInBuff].FileNumber = -1;
  if (!Aborted)
    FlushBuff(buffInfo);
  else if (buffInfo->OutNum != -1)
    FinalizeBuf(buffInfo);

  if (Parallel)
  {
    UninitBuf(wbuffInfo);
    ::CloseHandle(FlushEnd);
  }
  UninitBuf(buffInfo);

  if (FileCount)
    CopyProgressBox.Stop();
  else
  {
    FileNameStoreEnum Src(&SrcNames);
    for (size_t Index = 0; Index < Src.Count(); Index++)
    {
      if (!(Files[Index].Flags & FLG_COPIED))
      {
        if (Files[Index].Flags & FLG_DESCFILE)
          ProcessDesc(Index);
        else if (Move && Files[Index].Flags & FLG_DIR_POST &&
                 !(Files[Index].Flags & FLG_DIR_NOREMOVE))
        {
          if (RmDir(Src.GetByNum(Index)))
            Files[Index].Flags |= FLG_DELETED;
        }
      }
    }
  }
  buffInfo = nullptr;
  wbuffInfo = nullptr;
  ::CloseHandle(UiFree);
}

void Engine::ShowReadName(const String & fn)
{
  if (::WaitForSingleObject(UiFree, 0) == WAIT_TIMEOUT)
    return;
  CopyProgressBox.ShowReadName(fn);
  ::SetEvent(UiFree);
}

void Engine::ShowWriteName(const String & fn)
{
  if (::WaitForSingleObject(UiFree, 0) == WAIT_TIMEOUT)
    return;
  CopyProgressBox.ShowWriteName(fn);
  ::SetEvent(UiFree);
}

void Engine::ShowProgress(int64_t read, int64_t write, int64_t total,
                          int64_t readTime, int64_t writeTime,
                          int64_t readN, int64_t writeN, int64_t totalN)
{
  if (::WaitForSingleObject(UiFree, 0) == WAIT_TIMEOUT)
    return;

  CopyProgressBox.ShowProgress(read, write, total, readTime, writeTime,
                               readN, writeN, totalN, Parallel, FirstWrite,
                               StartTime, BufSize);
  ::SetEvent(UiFree);
}

intptr_t Engine::CheckOverwrite2(intptr_t fnum, const String & src, const String & dst, String & ren)
{
  ::WaitForSingleObject(UiFree, INFINITE);
  intptr_t res = CheckOverwrite(fnum, src, dst, ren);
  ::SetEvent(UiFree);
  return res;
}

void Engine::Delay(int64_t time, int64_t cb, volatile int64_t & counter, int64_t limit)
{
  if (limit <= 0)
    return;
  int64_t wait = (int64_t)((double)cb / limit * TicksPerSec()) - time;
  if (wait > 0)
  {
    Sleep((int)((double)wait / TicksPerSec() * 1000));
    counter += wait;
  }
}

void Engine::FarToWin32FindData(const TPanelItem & tpi, OUT WIN32_FIND_DATA & wfd)
{
  wfd.dwFileAttributes = (DWORD)tpi->FileAttributes;
  wfd.ftCreationTime = tpi->CreationTime;
  wfd.ftLastAccessTime = tpi->LastAccessTime;
  wfd.ftLastWriteTime = tpi->LastWriteTime;
  LARGE_INTEGER x;
  x.QuadPart = tpi->FileSize;
  wfd.nFileSizeHigh = x.HighPart;
  wfd.nFileSizeLow = x.LowPart;
}

String Engine::FindDescFile(const String & dir, intptr_t * idx)
{
  for (size_t Index = 0; Index < plugin->Descs().Count(); Index++)
  {
    if (FileExists(AddEndSlash(dir) + plugin->Descs()[Index]))
    {
      if (idx)
        *idx = Index;
      return plugin->Descs()[Index];
    }
  }
  if (idx)
  {
    *idx = -1;
  }
  return L"";
}

String Engine::FindDescFile(const String & dir, WIN32_FIND_DATA & fd, intptr_t * idx)
{
  for (size_t Index = 0; Index < plugin->Descs().Count(); Index++)
  {
    HANDLE DescFileHandle;
    if ((DescFileHandle = ::FindFirstFile((AddEndSlash(dir) + plugin->Descs()[Index]).ptr(), &fd)) != INVALID_HANDLE_VALUE)
    {
      ::FindClose(DescFileHandle);
      if (idx)
        *idx = Index;
      return plugin->Descs()[Index];
    }
  }
  if (idx)
    *idx = -1;
  return L"";
}

void Engine::AddTopLevelDir(const String & dir, const String & dstMask, DWORD flags, FileName::Direction d)
{
  HANDLE Handle;
  WIN32_FIND_DATA fd;

  SrcNames.AddRel(d, dir);
  DstNames.AddRel(d, ExtractFilePath(ApplyFileMaskPath(dir + L"\\somefile.txt", dstMask)));

  FileStruct info;
  ::ZeroMemory(&info, sizeof(info));
  info.Flags = flags;
  info.Level = 0;
  info.PanelIndex = -1;
  if ((Handle = ::FindFirstFile(dir.ptr(), &fd)) != INVALID_HANDLE_VALUE)
  {
    ::FindClose(Handle);
    info.Attr = fd.dwFileAttributes;
    info.creationTime = fd.ftCreationTime;
    info.lastAccessTime = fd.ftLastAccessTime;
    info.lastWriteTime = fd.ftLastWriteTime;
  }
  Files.push_back(info);
}

bool Engine::DirStart(const String & dir, const String & dstMask)
{
  if (_CopyDescs)
  {
    CurPathDesc = FindDescFile(dir, DescFindData);
  }
  CurPathFlags = CurPathAddFlags = 0;
  if (_ClearROFromCD && VolFlags(dir) & VF_CDROM)
  {
    CurPathAddFlags |= AF_CLEAR_RO;
  }
  AddTopLevelDir(dir, dstMask, FLG_DIR_PRE | FLG_DIR_FORCE | FLG_TOP_DIR | CurPathFlags, FileName::levelPlus);
  return true;
}

bool Engine::DirEnd(const String & dir, const String & dstMask)
{
  if (_CopyDescs && !CurPathDesc.IsEmpty())
  {
    if (!AddFile(AddEndSlash(dir) + CurPathDesc,
                 AddEndSlash(ExtractFilePath(ApplyFileMaskPath(AddEndSlash(dir) + CurPathDesc, dstMask))) + CurPathDesc, DescFindData, AF_DESCFILE | AF_DESC_INVERSE | CurPathAddFlags, 1)
       )
    {
      return false;
    }
  }
  AddTopLevelDir(dir, dstMask, FLG_DIR_POST | FLG_DIR_NOREMOVE | CurPathFlags, FileName::levelStar);
  return true;
}

static String getPanelDir(HANDLE h_panel)
{
  size_t bufSize = 512; // initial size

  std::unique_ptr<uint8_t[]> buf(new uint8_t[bufSize]);
  reinterpret_cast<FarPanelDirectory *>(buf.get())->StructSize = sizeof(FarPanelDirectory);
  size_t size = Info.PanelControl(h_panel, FCTL_GETPANELDIRECTORY, bufSize, buf.get());
  if (size > bufSize)
  {
    bufSize = size;
    buf.reset(new uint8_t[bufSize]);
    reinterpret_cast<FarPanelDirectory *>(buf.get())->StructSize = sizeof(FarPanelDirectory);
    Info.PanelControl(h_panel, FCTL_GETPANELDIRECTORY, bufSize, buf.get());
  }
  return reinterpret_cast<FarPanelDirectory *>(buf.get())->Name;
}

Engine::MResult Engine::Main(bool move, bool curOnly)
{
  PropertyMap & Options = plugin->Options();

  _CopyDescs    = (bool)Options[L"CopyDescs"];
  _DescsInDirs  = (bool)Options[L"DescsInSubdirs"];
  _ConfirmBreak = (bool)Options[L"ConfirmBreak"];
  _PreallocMin  = Options[L"PreallocMin"];
  _UnbuffMin    = Options[L"UnbuffMin"];

  _ClearROFromCD = true;
  _HideDescs = false;
  _UpdateRODescs = false;

  FarDialog & dlg = plugin->Dialogs()[L"CopyDialog"];
  dlg.ResetControls();
  FarDialog & advdlg = plugin->Dialogs()[L"AdvCopyDialog"];
  advdlg.ResetControls();
  CopyProgressBox.SetNeedToRedraw(true);

  String prompt, srcPath, dstPath;
  Move = move;
  int allowPlug = 0, adv = 0;

  String activePanelDir = getPanelDir(PANEL_ACTIVE);
  String passivePanelDir = getPanelDir(PANEL_PASSIVE);

  PanelInfo panel_info_passive;
  panel_info_passive.StructSize = sizeof(PanelInfo);
  Info.PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, &panel_info_passive); // !!! check result!

  PanelInfo panel_info_active;
  panel_info_active.StructSize = sizeof(PanelInfo);
  Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &panel_info_active); // !!! check result!

  if ((panel_info_passive.Flags & PFLAGS_PLUGIN) == PFLAGS_PLUGIN)
  {
    dstPath = L"plugin:";
    allowPlug = 1;
  }
  else
  {
    if (panel_info_passive.PanelType == PTYPE_QVIEWPANEL || panel_info_passive.PanelType == PTYPE_INFOPANEL ||  !(panel_info_passive.Flags & PFLAGS_VISIBLE))
    {
      dstPath.Clear();
    }
    else
    {
      if (Move)
      {
        if (activePanelDir == passivePanelDir)
        {
          if (panel_info_active.SelectedItemsNumber > 1 && !curOnly)
          {
            dstPath = AddEndSlash(passivePanelDir);
          }
          else
          {
            dstPath = L"";
          }
        }
        else
        {
          if (!curOnly)
          {
            dstPath = AddEndSlash(passivePanelDir);
          }
        }
      }
      else
      {
        if (!curOnly)
        {
          dstPath = AddEndSlash(passivePanelDir);
        }
      }
    }
  }

  if (panel_info_active.PanelType == PTYPE_QVIEWPANEL || panel_info_active.PanelType == PTYPE_INFOPANEL || !panel_info_active.ItemsNumber)
  {
    return MRES_NONE;
  }
  if ((panel_info_active.Flags & PFLAGS_REALNAMES) == 0)
    return MRES_STDCOPY;
  if (panel_info_active.SelectedItemsNumber > 1 && !curOnly)
  {
    if (move)
    {
      prompt = LOC(L"CopyDialog.MoveFilesTo");
    }
    else
    {
      prompt = LOC(L"CopyDialog.CopyFilesTo");
    }
  }
  else
  {
    bool pit_sel = panel_info_active.SelectedItemsNumber && !curOnly;
    TPanelItem pit(pit_sel ? 0 : panel_info_active.CurrentItem, true, pit_sel);

    String fileName = pit->FileName;
    String currentFileName = ExtractFileName(fileName);
    if (currentFileName == PARENTDIRECTORY)
      return MRES_NONE;
    String fmt;
    if (!move)
    {
      fmt = LOC(L"CopyDialog.CopyTo");
    }
    else
    {
      fmt = LOC(L"CopyDialog.MoveTo");
    }
    prompt = Format(fmt.ptr(), currentFileName.ptr());
    if (dstPath.empty())
      dstPath = currentFileName;

  }
  srcPath = CutEndSlash(activePanelDir);

  _InverseBars = (bool)Options[L"ConnectLikeBars"] && panel_info_active.PanelRect.left > 0;

  if (move)
  {
    dlg(L"Title") = LOC(L"CopyDialog.Move");
  }
  else
  {
    dlg(L"Title") = LOC(L"CopyDialog.Copy");
  }
  dlg[L"Label1"](L"Text") = prompt;
  dlg[L"DestPath"](L"Text") = dstPath;

  // Reset cp in case when user pressed Shift-F6/Shift-F5
  intptr_t cp = (!curOnly && !srcPath.IsEmpty()) ? CheckParallel(srcPath, dstPath) : FALSE;
  if (!Options[L"AllowParallel"])
  {
    dlg[L"ParallelCopy"](L"Selected") = FALSE;
  }
  else
  {
    if (cp == 1)
    {
      dlg[L"ParallelCopy"](L"Selected") = TRUE;
    }
    else
    {
      if (cp == 0)
      {
        dlg[L"ParallelCopy"](L"Selected") = FALSE;
      }
      else
      {
        dlg[L"ParallelCopy"](L"Selected") = Options[L"DefParallel"];
      }
    }
  }
  dlg[L"Ask"](L"Selected") = !Options[L"OverwriteDef"];
  dlg[L"Overwrite"](L"Selected") = Options[L"OverwriteDef"];

  if (WinNT)
  {
    advdlg[L"Streams"](L"Selected") = Options[L"CopyStreamsDef"];
    advdlg[L"Rights"](L"Selected") = Options[L"CopyRightsDef"];
  }
  else
  {
    advdlg[L"Streams"](L"Disable") = 1;
    advdlg[L"Rights"](L"Disable") = 1;
    advdlg[L"Compress"](L"Disable") = 1;
  }
  if (!Win2K)
  {
    advdlg[L"Encrypt"](L"Disable") = 1;
  }
  advdlg[L"creationTime"](L"Selected") = Options[L"copyCreationTime"];
  advdlg[L"lastAccessTime"](L"Selected") = Options[L"copyLastAccessTime"];
  advdlg[L"lastWriteTime"](L"Selected") = Options[L"copyLastWriteTime"];
  advdlg[L"Compress"](L"Selected") = Options[L"compressDef"];
  advdlg[L"Encrypt"](L"Selected") = Options[L"encryptDef"];
  advdlg[L"ReadSpeedLimit"](L"Selected") = Options[L"readSpeedLimitDef"];
  advdlg[L"ReadSpeedLimitValue"](L"Text") = Options[L"readSpeedLimitValueDef"];
  advdlg[L"WriteSpeedLimit"](L"Selected") = Options[L"writeSpeedLimitDef"];
  advdlg[L"WriteSpeedLimitValue"](L"Text") = Options[L"writeSpeedLimitLimitDef"];

  bool Repeat = true;
  while (Repeat)
  {
    Repeat = false;
    bool Cont = true;
    intptr_t dres = dlg.Execute();

    switch (dres)
    {
      case 2:
      {
        plugin->Config();
        Repeat = true;
        Cont = false;
        break;
      }

      case 1:
      {
        intptr_t advRes = advdlg.Execute();

        if (advRes == 1)
        {
          Options[L"copyCreationTime"] = advdlg[L"creationTime"](L"Selected");
          Options[L"copyLastAccessTime"] = advdlg[L"lastAccessTime"](L"Selected");
          Options[L"copyLastWriteTime"] = advdlg[L"lastWriteTime"](L"Selected");
          Options[L"compressDef"] = advdlg[L"Compress"](L"Selected");
          Options[L"encryptDef"] = advdlg[L"Encrypt"](L"Selected");
          Options[L"readSpeedLimitDef"] = advdlg[L"ReadSpeedLimit"](L"Selected");
          Options[L"readSpeedLimitValueDef"] = advdlg[L"ReadSpeedLimitValue"](L"Text");
          Options[L"writeSpeedLimitDef"] = advdlg[L"WriteSpeedLimit"](L"Selected");
          Options[L"writeSpeedLimitLimitDef"] = advdlg[L"WriteSpeedLimitValue"](L"Text");
          plugin->SaveOptions();
        }

        if (advRes == 0 || advRes == 1)
        {
          adv = 1;
          bool resume = advdlg[L"ResumeFiles"](L"Selected");
          dlg[L"Label2"](L"Disable") = resume;
          dlg[L"Ask"](L"Disable") = resume;
          dlg[L"Skip"](L"Disable") = resume;
          dlg[L"Overwrite"](L"Disable") = resume;
          dlg[L"Append"](L"Disable") = resume;
          dlg[L"Rename"](L"Disable") = resume;
          dlg[L"SkipIfNewer"](L"Disable") = resume;
          if (resume)
          {
            dlg[L"SkipIfNewer"](L"Selected") = 0;
          }
        }
        Repeat = true;
        Cont = false;
        break;
      }
      case -1:
      {
        return MRES_NONE;
      }
    }
    if (!Cont)
      break;

    String tmpDstText = dlg[L"DestPath"](L"Text");

    String dstText = tmpDstText.trim().trimquotes();

    if (!dstText.left(4).icmp(L"nul\\"))
      dstText = L"nul";

    dstText = dstText.replace(L"\"", L"");

    if (dstText == L"plugin:")
    {
      if (allowPlug)
      {
        return MRES_STDCOPY_RET;
      }
      else
      {
        ShowMessage(dlg(L"Text"), LOC(L"CopyDialog.InvalidPath"), FMSG_MB_OK);
        Repeat = true;
        break;
      }
    }

    String relDstPath = ExpandEnv(dstText.replace(L"/", L"\\"));
    dstPath.Clear();

    wchar_t CurrentDir[MAX_FILENAME];
    // Get absolute path for relative dstpath
    FSF.GetCurrentDirectory(_countof(CurrentDir), CurrentDir);
    // srcpath.ptr() for temporary file panel is empty
    if (relDstPath.icmp(L"nul") != 0)
    {
      dstPath = ConvertPath(CPM_REAL, relDstPath);
    }
    else
    {
      dstPath = relDstPath;
    }

    CompressMode = EncryptMode = ATTR_INHERIT;
    Streams = Rights = FALSE;

    if (WinNT)
    {
      Rights = advdlg[L"Rights"](L"Selected");
      Streams = advdlg[L"Streams"](L"Selected");
      CompressMode = advdlg[L"Compress"](L"Selected");
      if (Win2K)
      {
        EncryptMode = advdlg[L"Encrypt"](L"Selected");
        if (EncryptMode != ATTR_INHERIT)
          CompressMode = ATTR_INHERIT;
      }
    }

    copyCreationTime = advdlg[L"creationTime"](L"Selected");
    copyLastAccessTime = advdlg[L"lastAccessTime"](L"Selected");
    copyLastWriteTime = advdlg[L"lastWriteTime"](L"Selected");

    Parallel = dlg[L"ParallelCopy"](L"Selected");
    SkipNewer = dlg[L"SkipIfNewer"](L"Selected");
    SkippedToTemp = advdlg[L"SkippedToTemp"](L"Selected");
    ReadSpeedLimit = WriteSpeedLimit = 0;
    if ((bool)advdlg[L"ReadSpeedLimit"](L"Selected"))
    {
      ReadSpeedLimit = (int64_t)advdlg[L"ReadSpeedLimitValue"](L"Text") * 1024;
    }
    if ((bool)advdlg[L"WriteSpeedLimit"](L"Selected"))
    {
      WriteSpeedLimit = (int64_t)advdlg[L"WriteSpeedLimitValue"](L"Text") * 1024;
    }

    OverwriteMode = OM_PROMPT;
    if (advdlg[L"ResumeFiles"](L"Selected"))
    {
      OverwriteMode = OM_RESUME;
    }
    else if (dlg[L"Overwrite"](L"Selected"))
    {
      OverwriteMode = OM_OVERWRITE;
    }
    else if (dlg[L"Skip"](L"Selected"))
    {
      OverwriteMode = OM_SKIP;
    }
    else if (dlg[L"Append"](L"Selected"))
    {
      OverwriteMode = OM_APPEND;
    }
    else if (dlg[L"Rename"](L"Selected"))
    {
      OverwriteMode = OM_RENAME;
    }

    if (!dstPath.icmp(L"nul"))
    {
      OverwriteMode = OM_OVERWRITE;
      CompressMode = EncryptMode = ATTR_INHERIT;
      Streams = Rights = 0;
    }
    else
    {
      uint32_t vf = VolFlags(dstPath);
      if (vf != (uint32_t)-1)
      {
        if (!(vf & VF_STREAMS) && !adv)
          Streams = 0;
        if (!(vf & VF_RIGHTS) && !adv)
          Rights = 0;
        if (!(vf & VF_COMPRESSION) && !adv)
          CompressMode = ATTR_INHERIT;
        if (!(vf & VF_ENCRYPTION) && !adv)
          EncryptMode = ATTR_INHERIT;

        if (vf & VF_READONLY)
        {
          if (!warn(LOC(L"CopyDialog.ReadOnlyWarn")))
          {
            Repeat = true;
            break;
          }
        }
        if (!(vf & VF_COMPRESSION) && CompressMode == ATTR_ON)
        {
          if (!warn(LOC(L"CopyDialog.CompressWarn")))
          {
            Repeat = true;
            break;
          }
          CompressMode = ATTR_INHERIT;
        }
        if (!(vf & VF_ENCRYPTION) && EncryptMode == ATTR_ON)
        {
          if (!warn(LOC(L"CopyDialog.EncryptWarn")))
          {
            Repeat = true;
            break;
          }
          EncryptMode = ATTR_INHERIT;
        }
        if (!(vf & VF_STREAMS) && Streams)
        {
          if (!warn(LOC(L"CopyDialog.StreamsWarn")))
          {
            Repeat = true;
            break;
          }
          Streams = 0;
        }
        if (!(vf & VF_RIGHTS) && Rights == ATTR_ON)
        {
          if (!warn(LOC(L"CopyDialog.RightsWarn")))
          {
            Repeat = true;
            break;
          }
          Rights = 0;
        }
      }
      else
      {
        //if (!warn(LOC(L"CopyDialog.InvalidPathWarn")))
        ShowErrorMessage(LOC(L"CopyDialog.InvalidPathWarn"));
        Repeat = true;
        break;
      }
    }
  }

  // Check if srcpath=dstpath
  if (GetRealFileName(srcPath) == GetRealFileName(dstPath))
  {
    ShowErrorMessage(LOC(L"CopyDialog.OntoItselfError"));
    return MRES_NONE;
  }

  TotalBytes = 0;
  TotalN = 0;
  FileCount = CopyCount = 0;
  int64_t Start = GetTime();

  ScanFoldersProgressBox.ShowScanProgress(LOC(L"Status.ScanningFolders"));

  bool curItem = curOnly;
  if (!curItem)
  {
    curItem = !(TPanelItem(0, true, true)->Flags & PPIF_SELECTED);
  }

  std::vector<size_t> sortIndex;
  if (curItem)
  {
    sortIndex.push_back(panel_info_active.CurrentItem);
  }
  else
  {
    for (size_t Index = 0; Index < panel_info_active.ItemsNumber; Index++)
    {
      if (TPanelItem(Index)->Flags & PPIF_SELECTED)
      {
        sortIndex.push_back(Index);
      }
    }
  }

  String curPath;
  bool haveCurPath = 0;

  for (size_t Index = 0; Index < sortIndex.size(); Index++)
  {
    size_t Idx = sortIndex[Index];
    TPanelItem pit(Idx);
    String file = pit->FileName;
    if (file == PARENTDIRECTORY)
    {
      continue;
    }

    String filePath = ExtractFilePath(file);
    if (!haveCurPath || filePath.icmp(curPath))
    {
      if (haveCurPath && !DirEnd(!curPath.empty() ? curPath : srcPath, dstPath))
        return MainFinalize(panel_info_active);
      curPath = filePath;
      haveCurPath = 1;
      if (!DirStart(!curPath.empty() ? curPath : srcPath, dstPath))
        return MainFinalize(panel_info_active);
    }

    if (file.find('\\') == -1)
    {
      file = AddEndSlash(srcPath) + file;
    }

    if (_CopyDescs && !ExtractFileName(file).icmp(CurPathDesc))
    {
      if (ShowMessageEx(LOC(L"CopyDialog.Warning"),
                        FormatWidthNoExt(file, 50) + L"\n" +
                        LOC(L"CopyDialog.DescSelectedWarn") + L"\n" +
                        LOC(L"CopyDialog.DescSelectedWarn1"),
                        LOC(L"Framework.No") + L"\n" + LOC(L"Framework.Yes"), 0) != 1)
      {
        continue;
      }
      else
      {
        CurPathDesc.Clear();
      }
    }

    String dst = ApplyFileMaskPath(file, dstPath);
    WIN32_FIND_DATA wfd;
    FarToWin32FindData(pit, wfd);
    if (!AddFile(file, dst, wfd, CurPathAddFlags, 1, Idx))
    {
      return MainFinalize(panel_info_active);
    }
  }

  if (haveCurPath && !DirEnd(!curPath.empty() ? curPath : srcPath, dstPath))
    return MainFinalize(panel_info_active);

  if (OverwriteMode == OM_RESUME)
  {
    FileNameStoreEnum Enum(&DstNames);
    ScanFoldersProgressBox.ShowScanProgress(LOC(L"Status.ScanningFolders"));
    for (size_t Index = 0; Index < Enum.Count(); Index++)
    {
      if (!(Files[Index].Flags & FLG_SKIPPED) & !(Files[Index].Attr & FILE_ATTRIBUTE_DIRECTORY))
      {
        int64_t sz = FileSize(Enum.GetByNum(Index));
        if (sz < Files[Index].Size)
        {
          Files[Index].ResumePos = sz / ReadAlign * ReadAlign;
          TotalBytes -= Files[Index].ResumePos;
        }
        else
        {
          Files[Index].Flags |= FLG_SKIPPED;
          TotalBytes -= Files[Index].Size;
          TotalN--;
        }
      }
    }
  }
  else if (OverwriteMode == OM_SKIP || OverwriteMode == OM_RENAME || SkipNewer)
  {
    ScanFoldersProgressBox.ShowScanProgress(LOC(L"Status.ScanningFolders"));
    SetOverwriteMode(0);
  }

  ScanFoldersProgressBox.Hide();

  if (Options[L"BufPercent"])
  {
    BufSize = (size_t)(GetPhysMemorySize() / 100 * (int)Options[L"BufPercentVal"]);
  }
  else
  {
    BufSize = (size_t)((int64_t)Options[L"BufSizeVal"] * 1024);
  }

  if (Options[L"CheckFreeDiskSpace"])
  {
    if (!CheckFreeDiskSpace(TotalBytes, Move, srcPath, dstPath))
      return MRES_NONE; // not enough space
  }

  if (!CheckFATRestrictions(srcPath, dstPath))
    return MRES_NONE;

  if (CopyCount)
    Copy();

  if ((bool)Options[L"Sound"] && !Aborted)
  {
    if (GetTime() - Start > 30 * 60 * TicksPerSec())
      Beep(2);
    else if (GetTime() - Start > 10 * 60 * TicksPerSec())
      Beep(1);
    else if (GetTime() - Start > 30 * TicksPerSec())
      Beep(0);
  }

  return MainFinalize(panel_info_active);
}

Engine::MResult Engine::MainFinalize(PanelInfo & panel_info_active)
{
  ScanFoldersProgressBox.Hide();
  if (!Move)
  {
    Info.PanelControl(PANEL_ACTIVE, FCTL_BEGINSELECTION, 0, nullptr);
    for (size_t Index = 0; Index < Files.size(); Index++)
    {
      if (Files[Index].PanelIndex != -1)
      {
        if (Files[Index].Flags & FLG_DIR_PRE)
        {
          int ok = 1;
          size_t j = Index + 1;
          while (Files[j].Level > Files[Index].Level)
          {
            if (!(Files[j].Flags & (FLG_DIR_PRE | FLG_DIR_POST | FLG_COPIED)))
            {
              ok = 0;
              break;
            }
            j++;
          }
          if (ok)
          {
            Info.PanelControl(PANEL_ACTIVE, FCTL_SETSELECTION, Files[Index].PanelIndex, FALSE);
          }
          Index = j - 1;
        }
        else
        {
          if (!(Files[Index].Flags & FLG_DIR_POST) && Files[Index].Flags & FLG_COPIED)
          {
            Info.PanelControl(PANEL_ACTIVE, FCTL_SETSELECTION, Files[Index].PanelIndex, FALSE);
          }
        }
      }
    }
    Info.PanelControl(PANEL_ACTIVE, FCTL_ENDSELECTION, 0, nullptr);
  }
  else
  {
    Info.PanelControl(PANEL_ACTIVE, FCTL_UPDATEPANEL, 1, nullptr);
    Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &panel_info_active);

    PanelRedrawInfo rpi;
    rpi.TopPanelItem = panel_info_active.TopPanelItem;

    String NewFileName;
    for (size_t idx = 0; idx < Files.size(); idx++)
    {
      if (Files[idx].PanelIndex == panel_info_active.CurrentItem)
      {
        NewFileName = DstNames.GetNameByNum(idx);
        break;
      }
    }
    NewFileName = NewFileName.toLower();

    for (size_t Index = 0; Index < panel_info_active.ItemsNumber; Index++)
    {
      TPanelItem pit(Index);
      String NewPanelFilename = pit->FileName;
      NewPanelFilename = NewPanelFilename.toLower();
      if (NewFileName == NewPanelFilename)
      {
        rpi.CurrentItem = Index;
        Info.PanelControl(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, &rpi);
        break;
      }
    }
  }

  Info.PanelControl(PANEL_ACTIVE, FCTL_UPDATEPANEL, 1, nullptr);
  Info.PanelControl(PANEL_PASSIVE, FCTL_UPDATEPANEL, 1, nullptr);
  Info.PanelControl(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, nullptr);
  Info.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, nullptr);

  return MRES_OK;
}

bool Engine::AddFile(const String & Src, const String & Dst, WIN32_FIND_DATA & fd, DWORD Flags, intptr_t Level, intptr_t PanelIndex)
{
  return AddFile(Src, Dst, fd.dwFileAttributes, MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh), fd.ftCreationTime, fd.ftLastAccessTime, fd.ftLastWriteTime, Flags, Level, PanelIndex);
}

bool Engine::AddFile(const String & _src, const String & _dst, DWORD attr, int64_t size, const FILETIME & creationTime, const FILETIME & lastAccessTime, const FILETIME & lastWriteTime, DWORD flags, intptr_t Level, intptr_t PanelIndex)
{
  if (CheckEscape(false))
  {
    return false;
  }

  if (attr == INVALID_FILE_ATTRIBUTES)
  {
    return true;
  }

  String src(_src);
  String dst(_dst);

  // Get here the real file names
  // unfold symlinks in file paths (if any)
  // src path
//  DWORD _src_attr = ::GetFileAttributes(ExtractFilePath(_src).ptr());
//  if ((_src_attr != 0xFFFFFFFF) &&
//          (_src_attr & FILE_ATTRIBUTE_DIRECTORY) &&
//          (_src_attr & FILE_ATTRIBUTE_REPARSE_POINT))
//      src = AddEndSlash(GetRealFileName(ExtractFilePath(_src))) + ExtractFileName(_src);
//  else
//      src = _src;
//
//  // dst path
//  DWORD _dst_attr = ::GetFileAttributes(ExtractFilePath(_dst).ptr());
//  if ((_dst_attr != 0xFFFFFFFF) &&
//      (_dst_attr & FILE_ATTRIBUTE_DIRECTORY) &&
//      (_dst_attr & FILE_ATTRIBUTE_REPARSE_POINT))
//      dst = AddEndSlash(GetRealFileName(ExtractFilePath(_dst))) + ExtractFileName(_dst);
//  else
//      dst = _dst;
  //////////////////////////////////////////////////////

  if (src == dst && !(flags & AF_DESCFILE))
  {
    return true;
  }

  if (flags & AF_CLEAR_RO)
  {
    attr &= ~FILE_ATTRIBUTE_READONLY;
  }

  int64_t sz1 = (attr & FILE_ATTRIBUTE_DIRECTORY) ? 0 : size;

  FileStruct fs;
  ::ZeroMemory(&fs, sizeof(fs));
  std::vector<FileStruct>::iterator it = Files.insert(Files.end(), fs);

  it->Size = sz1;
  it->Attr = attr;
  it->creationTime = creationTime;
  it->lastAccessTime = lastAccessTime;
  it->lastWriteTime = lastWriteTime;

  it->Level = Level;
  it->PanelIndex = PanelIndex;

  FileName::Direction d;
  if (attr & FILE_ATTRIBUTE_DIRECTORY)
  {
    it->Flags |= FLG_DIR_PRE;
    d = FileName::levelPlus;
  }
  else
  {
    d = FileName::levelSame;
  }

  SrcNames.AddRel(d, ExtractFileName(src));
  DstNames.AddRel(d, ExtractFileName(dst));

  if (flags & AF_DESCFILE)
  {
    it->Flags |= FLG_DESCFILE;
    if (flags & AF_DESC_INVERSE)
    {
      it->Flags |= FLG_DESC_INVERSE;
    }
    CopyCount++;
    return true;
  }

  uint32_t owmode = OM_PROMPT;
  if (!(flags & AF_STREAM))
  {
    if (Move)
    {
      bool Retry = true;
      while (Retry)
      {
        Retry = false;
        if (FMoveFile(src, dst, false))
        {
          it->Flags |= FLG_COPIED | FLG_DELETED;
          AddFile2(src, Level, attr, dst);
          return true;
        }
        DWORD err = ::GetLastError();
        if (err == ERROR_ALREADY_EXISTS && !(attr & FILE_ATTRIBUTE_DIRECTORY))
        {
          if (OverwriteMode == OM_RESUME)
          {
            if (FileSize(dst) >= it->Size)
            {
              it->Flags |= FLG_SKIPPED;
              AddFile2(src, Level, attr, dst);
              return true;
            }
          }
          else
          {
            String ren;
            intptr_t res, j = 0;
            if (SkipNewer && Newer(dst, lastWriteTime))
            {
              res = OM_SKIP;
            }
            else if (OverwriteMode == OM_PROMPT)
            {
              res = CheckOverwrite(-1, src, dst, ren);
            }
            else
            {
              res = OverwriteMode;
            }
            switch (res)
            {
              case OM_SKIP:
                it->Flags |= FLG_SKIPPED;
                AddFile2(src, Level, attr, dst);
                return true;
                break;

              case OM_OVERWRITE:
                owmode = OM_OVERWRITE;
                if (FMoveFile(src, dst, true))
                {
                  it->Flags |= FLG_COPIED | FLG_DELETED;
                  AddFile2(src, Level, attr, dst);
                  return true;
                }
                break;

              case OM_APPEND:
                owmode = OM_APPEND;
                break;

              case OM_RENAME:
                if (OverwriteMode != OM_RENAME)
                {
                  dst = ren;
                }
                else
                {
                  while (ExistsN(dst, j))
                    j++;
                  dst = DupName(dst, j);
                }
                Retry = true;
                break;

              case OM_CANCEL:
                return false;
                break;
            }
          }
        }
      }
    }
  }

  CopyCount++;
  TotalN++;
  if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
  {
    FileCount++;
  }
  TotalBytes += sz1;
  it->OverMode = owmode;

  // Updates folder scan progress dialog
  ScanFoldersProgressBox.SetScanProgressInfo(TotalN, TotalBytes);

  if (!(flags & AF_STREAM))
  {
    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
      if (attr & FILE_ATTRIBUTE_REPARSE_POINT)
      {
        if (_ClearROFromCD && VolFlags(src) & VF_CDROM)
          flags |= AF_CLEAR_RO;
      }
      WIN32_FIND_DATA fd;
      HANDLE Handle;
      String SrcDir = GetLongFileName(src) + L"\\*.*";
      if ((Handle = ::FindFirstFile(SrcDir.ptr(), &fd)) != INVALID_HANDLE_VALUE)
      {
        intptr_t descidx = -1;
        RememberStruct Remember;
        while (1)
        {
          if ((wcscmp(fd.cFileName, PARENTDIRECTORY) != 0) && (wcscmp(fd.cFileName, THISDIRECTORY) != 0))
          {
            intptr_t idx;
            if (_CopyDescs && _DescsInDirs &&
                !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (idx = plugin->Descs().Find(fd.cFileName)) != -1)
            {
              if (descidx == -1 || idx < descidx)
              {
                RememberFile(AddEndSlash(src) + fd.cFileName, AddEndSlash(dst) + fd.cFileName,
                             fd, flags | AF_DESCFILE, Level + 1, Remember);
                descidx = idx;
              }
            }
            else
            {
              // check for recursive symlinks
              // recursive symlinks results in stack overflow :(
              if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                  (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
              {
                // compare current folder & unfolded symlink name in current folder
                if (GetRealFileName(src) != GetRealFileName(AddEndSlash(src) + fd.cFileName))
                {
                  if (!AddFile(AddEndSlash(src) + fd.cFileName, AddEndSlash(dst) + fd.cFileName, fd, flags, Level + 1))
                  {
                    ::FindClose(Handle);
                    return false;
                  }
                }
              }
              else
              {
                if (!AddFile(AddEndSlash(src) + fd.cFileName, AddEndSlash(dst) + fd.cFileName, fd, flags, Level + 1))
                {
                  ::FindClose(Handle);
                  return false;
                }
              }
            }
          }
          if (!::FindNextFile(Handle, &fd))
            break;
        }
        ::FindClose(Handle);
        if (descidx != -1)
          if (!AddRemembered(Remember))
            return false;
      }
    }
    else if (Streams)
    {
      WIN32_STREAM_ID sid;
      ::ZeroMemory(&sid, sizeof(sid));

      HANDLE Handle = ::CreateFile(src.ptr(), GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                             OPEN_EXISTING, 0, nullptr);
      if (Handle != INVALID_HANDLE_VALUE)
      {
        DWORD hsz = offsetof(WIN32_STREAM_ID, cStreamName);
        LPVOID ctx = nullptr;
        while (1)
        {
          wchar_t strn[1024];
          DWORD cb1, cb2;
          if (!::BackupRead(Handle, (LPBYTE)&sid, hsz, &cb1, FALSE, FALSE, &ctx) || !cb1)
            break;
          strn[0] = 0;
          if (sid.dwStreamNameSize)
            if (!::BackupRead(Handle, (LPBYTE)strn, sid.dwStreamNameSize, &cb2, FALSE, FALSE, &ctx) ||
                !cb2)
              break;
          if ((sid.dwStreamId == BACKUP_DATA ||
               sid.dwStreamId == BACKUP_EA_DATA ||
               sid.dwStreamId == BACKUP_ALTERNATE_DATA) &&
              sid.dwStreamNameSize)
          {
            strn[sid.dwStreamNameSize / 2] = 0;
            if (!AddFile(src + strn, dst + strn, attr,
                         sid.Size.QuadPart, creationTime, lastAccessTime, lastWriteTime, flags | AF_STREAM, Level + 1))
            {
              ::BackupRead(nullptr, nullptr, 0, nullptr, TRUE, FALSE, &ctx);
              ::CloseHandle(Handle);
              return false;
            }
          }
          ::BackupSeek(Handle, sid.Size.LowPart, sid.Size.HighPart,
                     &cb1, &cb2, &ctx);
        }
        ::BackupRead(nullptr, nullptr, 0, nullptr, TRUE, FALSE, &ctx);
        ::CloseHandle(Handle);
      }
    }
  }

  AddFile2(src, Level, attr, dst);
  return true;
}

void Engine::AddFile2(const String & src, intptr_t Level, DWORD attr, const String & dst)
{
  if (attr & FILE_ATTRIBUTE_DIRECTORY)
  {
    FileStruct info;
    ::ZeroMemory(&info, sizeof(info));
    info.Flags |= FLG_DIR_POST | info.Flags & (FLG_COPIED | FLG_DELETED);
    info.Level = Level;
    info.PanelIndex = -1;
    Files.push_back(info);

    SrcNames.AddRel(FileName::levelMinus, ExtractFileName(src));
    DstNames.AddRel(FileName::levelMinus, ExtractFileName(dst));
  }
}

void Engine::SetOverwriteMode(intptr_t Start)
{
  FileNameStoreEnum Enum(&DstNames);
  for (size_t Index = Start; Index < Enum.Count(); Index++)
  {
    String fn = Enum.GetByNum(Index);
    FileStruct & info = Files[Index];
    if (!SkipNewer && info.Flags & FLG_SKIPNEWER)
    {
      info.Flags &= ~(FLG_SKIPNEWER | FLG_SKIPPED);
      TotalBytes += info.Size;
      TotalN++;
    }
    if (!(info.Flags & FLG_SKIPPED) && !(info.Flags & FLG_DIR_PRE) && !(info.Flags & FLG_DIR_POST) && FileExists(fn))
    {
      if (SkipNewer && Newer(fn, info.lastWriteTime))
      {
        TotalBytes -= info.Size;
        TotalN--;
        info.Flags |= FLG_SKIPPED | FLG_SKIPNEWER;
      }
      else
      {
        switch (OverwriteMode)
        {
          case OM_SKIP:
          {
            TotalBytes -= info.Size;
            TotalN--;
            info.Flags |= FLG_SKIPPED;
            break;
          }
          case OM_RENAME:
          {
            intptr_t j = 0;
            while (ExistsN(fn, j))
              j++;
            info.RenameNum = j;
            break;
          }
        }
      }
    }
  }
}

intptr_t Engine::CheckOverwrite(intptr_t fnum, const String & Src, const String & Dst, String & ren)
{
  FarDialog & dlg = plugin->Dialogs()[L"OverwriteDialog"];
  dlg.ResetControls();
  CopyProgressBox.SetNeedToRedraw(true);

  dlg[L"Label2"](L"Text") = Dst;
  String ssz, dsz, stime, dtime, buf;

  WIN32_FIND_DATA fd, fs;
  ::FindClose(::FindFirstFile(Dst.ptr(), &fd));
  dsz = FormatNum(MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh));
  dtime = FormatTime(fd.ftLastWriteTime);
  dlg[L"Label4"](L"Text") = Format(L"%14s %s %s", dsz.ptr(), LOC(L"OverwriteDialog.Bytes").ptr(), dtime.ptr());

  ::FindClose(::FindFirstFile(Src.ptr(), &fs));
  ssz = FormatNum(MAKEINT64(fs.nFileSizeLow, fs.nFileSizeHigh));
  stime = FormatTime(fs.ftLastWriteTime);
  dlg[L"Label3"](L"Text") = Format(L"%14s %s %s", ssz.ptr(), LOC(L"OverwriteDialog.Bytes").ptr(), stime.ptr());

  dlg[L"Source"](L"Text") = Format(L"%-16s", LOC(L"OverwriteDialog.Source").ptr());
  dlg[L"Destination"](L"Text") = Format(L"%-16s", LOC(L"OverwriteDialog.Destination").ptr());

  dlg[L"AcceptForAll"](L"Selected") = 0;
  dlg[L"SkipIfNewer"](L"Selected") = SkipNewer;
  dlg[L"SkippedToTemp"](L"Selected") = SkippedToTemp;
  bool OldSkipNewer = SkipNewer;

  intptr_t res = OM_PROMPT, ores = -1;
  int AcceptForAll = dlg[L"AcceptForAll"](L"Selected");
  bool Repeat = true;
  while (Repeat)
  {
    Repeat = false;
    switch (dlg.Execute())
    {
      case 0:
        res = OM_OVERWRITE;
        break;
      case 1:
        res = OM_SKIP;
        break;
      case 2:
        res = OM_APPEND;
        break;
      case 3:
        res = OM_RENAME;
        break;
      case -1:
        return OM_CANCEL;
    }

    AcceptForAll = dlg[L"AcceptForAll"](L"Selected");
    SkipNewer = dlg[L"SkipIfNewer"](L"Selected");
    SkippedToTemp = dlg[L"SkippedToTemp"](L"Selected");

    if (SkipNewer && Newer(fd.ftLastWriteTime, fs.ftLastWriteTime))
    {
      ores = res;
      res = OM_SKIP;
    }
    Repeat = false;

    if (res == OM_RENAME && !AcceptForAll)
    {
      FarDialog & dlg = plugin->Dialogs()[L"RenameDialog"];
      dlg.ResetControls();
      CopyProgressBox.SetNeedToRedraw(true);

      dlg[L"Edit"](L"Text") = ExtractFileName(Dst);
      bool Repeat1 = true;
      while (Repeat1)
      {
        Repeat1 = false;
        if (dlg.Execute() == 0)
        {
          ren = AddEndSlash(ExtractFilePath(Dst)) + dlg[L"Edit"](L"Text");
          if (ExistsN(ren, 0))
            Repeat1 = true;
        }
        else
        {
          Repeat = true;
        }
      }
    }
  }

  if (AcceptForAll || SkipNewer != OldSkipNewer)
  {
    if (AcceptForAll)
      if (ores == -1)
        OverwriteMode = res;
      else
        OverwriteMode = ores;
    if (fnum != -1)
    {
      SetOverwriteMode(fnum + 1);
      if (res == OM_RENAME)
      {
        intptr_t j = 0;
        while (ExistsN(Dst, j))
          j++;
        ren = DupName(Dst, j);
      }
    }
  }

  return res;
}

void Engine::RememberFile(const String & Src, const String & Dst,
                          WIN32_FIND_DATA & fd, DWORD Flags, intptr_t Level,
                          RememberStruct & Remember)
{
  Remember.Src = Src;
  Remember.Dst = Dst;
  Remember.Attr = fd.dwFileAttributes;
  Remember.Size = MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh);
  Remember.creationTime = fd.ftCreationTime;
  Remember.lastAccessTime = fd.ftLastAccessTime;
  Remember.lastWriteTime = fd.ftLastWriteTime;
  Remember.Flags = Flags;
  Remember.Level = Level;
}

bool Engine::AddRemembered(RememberStruct & Remember)
{
  return AddFile(Remember.Src, Remember.Dst, Remember.Attr, Remember.Size, Remember.creationTime, Remember.lastAccessTime, Remember.lastWriteTime, Remember.Flags, Remember.Level);
}

bool Engine::CheckFreeDiskSpace(int64_t TotalBytesToProcess, bool MoveMode,
                                const String & srcpathstr, const String & dstpathstr)
{
  String srcroot = GetFileRoot(srcpathstr);
  String dstroot = GetFileRoot(dstpathstr);

  if ((MoveMode) && (srcroot == dstroot))
    return true;

  bool result = false;

  ULARGE_INTEGER FreeBytesAvailable;
  ULARGE_INTEGER TotalNumberOfBytes;
  ULARGE_INTEGER TotalNumberOfFreeBytes;

  if (::GetDiskFreeSpaceEx(dstroot.ptr(), &FreeBytesAvailable,
                         &TotalNumberOfBytes, &TotalNumberOfFreeBytes))
  {
    if (FreeBytesAvailable.QuadPart < (ULONGLONG)TotalBytesToProcess)
    {
      ::WaitForSingleObject(UiFree, INFINITE);

      FarDialog & dlg = plugin->Dialogs()[L"FreeSpaceErrorDialog"];
      dlg.ResetControls();
      CopyProgressBox.SetNeedToRedraw(true);

      dlg(L"Title") = LOC(L"FreeSpaceErrorDialog.Title");
      String disk_str = dstroot;
      if (disk_str.len() >= 2)
        if (disk_str[1] == L':')
          disk_str = disk_str.left(2);
      dlg[L"Label1"](L"Text") = LOC(L"FreeSpaceErrorDialog.NotEnoughSpace") + L" " + disk_str;
      dlg[L"Label2"](L"Text") = Format(L"%-20s%12s", LOC(L"FreeSpaceErrorDialog.AvailableSpace").ptr(),
                                     FormatValue(FreeBytesAvailable.QuadPart).ptr());
      dlg[L"Label3"](L"Text") = Format(L"%-20s%12s", LOC(L"FreeSpaceErrorDialog.RequiredSpace").ptr(),
                                     FormatValue(TotalBytesToProcess).ptr());
      dlg[L"Label4"](L"Text") = LOC(L"FreeSpaceErrorDialog.AbortPrompt");

      intptr_t dlgres = dlg.Execute();

      HANDLE h = ::GetStdHandle(STD_INPUT_HANDLE);
      ::FlushConsoleInputBuffer(h);
      ::SetEvent(UiFree);

      if (dlgres == RES_YES)
        result = false;
      else
        result = true;
    }
    else
      result = true;
  }
  else
    result = true;

  return result;
}

bool Engine::CheckFATRestrictions(const String & srcpathstr, const String & dstpathstr)
{
  if (!IsFAT(dstpathstr))
    return true;
  bool Result = true;
  // check for big files
  FileNameStoreEnum Src(&SrcNames);
  for (size_t Index = 0; Index < Src.Count(); Index++)
  {
    FileStruct & info = Files[Index];
    if (info.Flags & FLG_SKIPPED ||
        info.Flags & FLG_COPIED ||
        info.Flags & FLG_DESCFILE)
      continue;
    if (info.Size >= (int64_t)4 * 1024 * 1024 * 1024)
    {
      ::WaitForSingleObject(UiFree, INFINITE);

      FarDialog & dlg = plugin->Dialogs()[L"FATErrorDialog"];
      dlg.ResetControls();
      CopyProgressBox.SetNeedToRedraw(true);

      dlg(L"Title") = LOC(L"FATErrorDialog.Title");
      String disk_str = GetFileRoot(dstpathstr);
      if (disk_str.len() >= 2)
        if (disk_str[1] == L':')
          disk_str = disk_str.left(2);
      String fmt = LOC(L"FATErrorDialog.Message");
      String FileName = SrcNames.GetNameByNum(Index);
      String msg = Format(fmt.ptr(), FileName.ptr());
      dlg[L"Label1"](L"Text") = msg + L" " + disk_str;
      dlg[L"Label2"](L"Text") = LOC(L"FATErrorDialog.AbortPrompt");

      intptr_t dlgres = dlg.Execute();

      HANDLE h = ::GetStdHandle(STD_INPUT_HANDLE);
      ::FlushConsoleInputBuffer(h);
      ::SetEvent(UiFree);

      if (dlgres == RES_YES)
        Result = false;
      else
        Result = true;

      break;
    }
  }
  return Result;
}

#define check(a, b) a ? b : nullptr

void Engine::SetFileTime(HANDLE h, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime)
{
  SetFileTime2(h, check(copyCreationTime, creationTime), check(copyLastAccessTime, lastAccessTime), check(copyLastWriteTime, lastWriteTime));
}

void Engine::SetFileSizeAndTime(const String & fn, int64_t size, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime)
{
  SetFileSizeAndTime2(fn, size, check(copyCreationTime, creationTime), check(copyLastAccessTime, lastAccessTime), check(copyLastWriteTime, lastWriteTime));
}

intptr_t Engine::EngineError(const String & s, const String & fn, int code, uint32_t & flg, const String & title, const String & type_id)
{
  CopyProgressBox.SetNeedToRedraw(true);
  ScanFoldersProgressBox.SetNeedToRedraw(true);
  int * ix = nullptr;
  if (flg & eeAutoSkipAll)
  {
    ix = &errTypes[type_id];
    if (*ix & errfSkipAll)
    {
      if (flg & eeShowKeepFiles && *ix & errfKeepFiles)
      {
        flg |= eerKeepFiles;
      }
      return RES_SKIP;
    }
  }

  FarDialog & dlg = plugin->Dialogs()[L"CopyError"];
  dlg.ResetControls();
  CopyProgressBox.SetNeedToRedraw(true);

  if (!title.empty())
  {
    dlg(L"Title") = title;
  }

  if (flg & eeShowReopen)
  {
    dlg[L"Reopen"](L"Visible") = 1;
  }
  if (flg & eeShowKeepFiles)
  {
    dlg[L"KeepFiles"](L"Visible") = 1;
  }
  if (flg & eeYesNo)
  {
    dlg[L"YesNo"](L"Visible") = 1;
  }
  if (flg & eeRetrySkipAbort)
  {
    dlg[L"RetrySkipAbort"](L"Visible") = 1;
    if (flg & eeAutoSkipAll)
    {
      dlg[L"SkipAll"](L"Visible") = 1;
    }
  }
  dlg[L"Label1"](L"Text") = s;
  dlg[L"Label1"](L"Visible") = 1;
  if (flg & eeOneLine)
  {
    dlg[L"Sep1"](L"Visible") = 0;
  }
  else
  {
    dlg[L"Label2"](L"Text") = FormatWidthNoExt(fn, msgw());
    dlg[L"Label2"](L"Visible") = 1;
    StringVector list;
    list.loadFromString(SplitWidth(GetErrText(code), msgw()), '\n');
    for (size_t Index = 0; Index < list.Count(); Index++)
    {
      if (Index <= 7)
      {
        String name = String(L"Label") + String(Index + 3);
        dlg[name](L"Visible") = 1;
        dlg[name](L"Text") = list[Index];
      }
    }
  }

  if (!(flg & eeShowReopen) && !(flg & eeShowKeepFiles))
  {
    dlg[L"Sep1"](L"Visible") = 0;
  }

  intptr_t res = dlg.Execute();

  if ((bool)dlg[L"Reopen"](L"Selected"))
    flg |= eerReopen;
  if ((bool)dlg[L"KeepFiles"](L"Selected"))
    flg |= eerKeepFiles;

  if (flg & eeYesNo)
  {
    if (res == 0)
      return RES_YES;
    if (res == -1)
      return RES_NO;
  }
  else if (flg & eeRetrySkipAbort)
  {
    if (res == 0)
      return RES_RETRY;
    if (res == 1)
      return RES_SKIP;
    if (res == -1)
      return RES_ABORT;
    if (res == 2 && ix)
    {
      *ix |= errfSkipAll;
      if (flg & eerKeepFiles)
      {
        *ix |= errfKeepFiles;
      }
      return RES_SKIP;
    }
  }
  return -1;
}

void Engine::FWError2(const String & Msg)
{
  ::WaitForSingleObject(UiFree, INFINITE);
  FWError(Msg);
  ::SetEvent(UiFree);
}

TPanelItem::TPanelItem(size_t idx, bool active, bool selected)
{
  ppi = GetPanelItem(
          active ? PANEL_ACTIVE : PANEL_PASSIVE,
          selected ? FCTL_GETSELECTEDPANELITEM : FCTL_GETPANELITEM,
          idx
        );
}
