#include "../framework/stdhdr.h"
#include "../framework/lowlevelstr.h"
#include "filecopyex.h"
#include "engine.h"
#include "common.h"
#include "../framework/fileutils.h"
#include "../framework/strutils.h"

#include "enginetools.h"
#include "../framework/FwCommon.h"

#define AllocAlign  65536

// Bug #10 fixed by Ivanych
//#define ReadBlock   (4 << 20)
//#define WriteBlock  (4 << 20)

#define ReadBlock   0x20000     // 128 KB
#define WriteBlock  0x20000     // 128 KB

Engine::Engine(): FlushSrc(SrcNames), FlushDst(DstNames)
{
  Parallel=Streams=Rights=Move=SkipNewer=SkippedToTemp=0;
  CompressMode=EncryptMode=ATTR_INHERIT;
  OverwriteMode=OM_PROMPT;
  BufSize=0;

  SYSTEM_INFO si;
  GetSystemInfo(&si);
  ReadAlign=si.dwPageSize;

//  errTypes.SetOptions(slSorted | slIgnoreCase);
}

int Engine::InitBuf(BuffInfo *bi)
{
  bi->OutFile=NULL;
  bi->OutNum=-1;

  bi->BuffSize=BufSize;
  if (Parallel) bi->BuffSize/=2;
  bi->BuffSize=(bi->BuffSize/AllocAlign+1)*AllocAlign;

  bi->Buffer = (UCHAR*)Alloc(bi->BuffSize);
  if (!bi->Buffer)
  {
    Error(LOC("Error.MemAlloc"), GetLastError());
    return FALSE;
  }
  bi->BuffInf = (BuffStruct*)Alloc(SrcNames.Count()*sizeof(BuffStruct));
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
  String 
    &DstName=bi->DstName,   
    &SrcName=bi->SrcName;
  FileStruct &info=Files[fnum];

  if (!(info.Flags & FLG_SKIPPED))
  {
    if (info.Flags & FLG_BUFFERED)
      SetFileTime(Handle, info.Modify);
  }

  Close(Handle);

  if (!(info.Flags & FLG_SKIPPED))
  {
    info.Flags|=FLG_COPIED;
    LastFile=fnum;

    if (!(info.Flags & FLG_BUFFERED))
    {
      if (info.OverMode==OM_APPEND)
        SetFileSizeAndTime(DstName, bi->OrgSize+info.Size, info.Modify);
      else
        SetFileSizeAndTime(DstName, info.Size, info.Modify);
    }

    if (Rights) CopyACL(SrcName, DstName);
    Encrypt(DstName, EncryptMode);
    SetFileAttributes(DstName.ptr(), info.Attr);

    if (Move)
    {
del_retry:
      // Bug #6 fixed by CDK
      if (FileExists(SrcName) && !Delete(SrcName))
      {
        WaitForSingleObject(UiFree, INFINITE);
        int flg=eeRetrySkipAbort | eeAutoSkipAll,
            res=EngineError(LOC("Error.FileDelete"), SrcName, GetLastError(),
            flg, "", "Error.FileDelete");
        SetEvent(UiFree);
        if (res==RES_RETRY) goto del_retry;
        else if (res==RES_ABORT) Aborted=1;
      }
      else
        info.Flags |= FLG_DELETED;
    }

    WriteN++;

    int dnum=fnum+1;
    while (dnum<SrcNames.Count() 
      && Files[dnum].Flags & (FLG_DIR_POST | FLG_DIR_PRE | FLG_DESCFILE))
    {
      if (!(Files[dnum].Flags & FLG_COPIED))
      {
        if (Files[dnum].Flags & FLG_DESCFILE)
          ProcessDesc(dnum);
        else if (Files[dnum].Flags & FLG_DIR_POST 
          && !(Files[dnum].Flags & FLG_DIR_NOREMOVE) && Move)
        {
          if (RmDir(FlushSrc.GetByNum(dnum)))
            Files[dnum].Flags |= FLG_COPIED | FLG_DELETED;
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

    if (Handle)
    {
      if (info.OverMode==OM_APPEND || info.OverMode==OM_RESUME)
      {
        if (KeepFiles || (info.Flags & FLG_KEEPFILE)) 
          SetFileSizeAndTime(DstName, bi->OrgSize+info.Written, info.Modify);
        else
          SetFileSizeAndTime(DstName, bi->OrgSize, info.Modify);
      }
      else 
      {
        if (KeepFiles || (info.Flags & FLG_KEEPFILE)) 
          SetFileSizeAndTime(DstName, info.Written, info.Modify);
        else
          Delete(DstName);
      }
      SetFileAttributes(DstName.ptr(), info.Attr);
    }
  }
}

void Engine::ProcessDesc(int fnum)
{
  String DstName=FlushDst.GetByNum(fnum), 
         SrcName=FlushSrc.GetByNum(fnum);
  //TODO: OM_RENAME can change DstName
  FileStruct &info=Files[fnum];

  String DstPath=AddEndSlash(ExtractFilePath(DstName));
  String df=FindDescFile(DstPath);
  if (df!="") DstName=DstPath+df;

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
      String sn=SrcNames.GetNameByNum(j),
             dn=DstNames.GetNameByNum(j);
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
            FWError(Format("FSeek to %d failed, code %d", info.ResumePos, 
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

          __int64 size=info.OverMode==OM_APPEND?bi->OrgSize+info.Size:info.Size;
          if (size>=_PreallocMin*1024 
            && GetCompression(bi->OutFile)==0)
          {
            __int64 sp;
            if (!(info.Flags & FLG_BUFFERED))
            {
              sp=size/info.SectorSize;
              if (size%info.SectorSize) sp++;
              sp*=info.SectorSize;
            }
            else sp=size;
            __int64 bp=FTell(bi->OutFile);
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

        int wsz = (int)min(bi->BuffInf[PosInStr].WritePos - Pos, WriteBlock),
            wsz1 = wsz;
        if (info.Flags & FLG_BUFFERED)
          wsz = (int)min(wsz, info.Size-info.Written);
retry:
        __int64 st = GetTime();
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
              __int64 Pos=info.Written;
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
                FWError(Format("FSeek to %d failed, code %d", Pos, 
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

        __int64 wt = GetTime()-st;
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

void __stdcall FlushThread(Engine* eng)
{
  eng->FlushBuff(eng->wbi);
}

void Engine::BGFlush()
{
  DWORD id;
  BGThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FlushThread, this, 0, &id);
}

int Engine::WaitForFlushEnd()
{
  while (WaitForSingleObject(FlushEnd, 500) == WAIT_TIMEOUT)
  {
    if (CheckEscape()) return FALSE;
  }
  return TRUE;
}

struct CurDirInfo
{
  int SectorSize;
};

void Engine::Copy()
{
  LastFile=-1;
  Aborted=KeepFiles=0;
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

  FileNameStoreEnum Src(SrcNames);
  FileNameStoreEnum Dst(DstNames);
  Array<CurDirInfo> CurDirStack;

  for (int i=0; i<Src.Count(); i++)
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
            if (!(info.Flags & FLG_DIR_FORCE))
              SetFileTime(hd, NULL, NULL, &info.Modify);
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
      if (!CurDirStack.Count() || dattr != 0xFFFFFFFF 
        && dattr & FILE_ATTRIBUTE_REPARSE_POINT)
          cdi.SectorSize=GetSectorSize(DstName);
      else
        cdi.SectorSize=
          CurDirStack[CurDirStack.Count()-1].SectorSize;
      CurDirStack.Add(cdi);
      SectorSize=cdi.SectorSize;
      continue;
    } // if (info.Flags & FLG_DIR_PRE) 

    if (info.Flags & FLG_DIR_POST) 
    {
      CurDirStack.Delete(CurDirStack.Count()-1);
	  if(CurDirStack.Count())
	      SectorSize=CurDirStack[CurDirStack.Count()-1].SectorSize;
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
        FWError(Format("FSeek to %d failed, code %d", info.ResumePos, 
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
        int j, cb = min(ReadBlock, bi->BuffSize-BuffPos);
retry:
        __int64 st = GetTime();
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
              __int64 Pos=info.Read;
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
                FWError(Format("FSeek to %d failed, code %d", Pos, 
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

        __int64 rt = GetTime()-st;
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
    FileNameStoreEnum Src(SrcNames);
    for (int i=0; i<Src.Count(); i++)
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

void Engine::ShowProgress(__int64 read, __int64 write, __int64 total,
                          __int64 readTime, __int64 writeTime,
                          __int64 readN, __int64 writeN, __int64 totalN)
{
  if (WaitForSingleObject(UiFree, 0) == WAIT_TIMEOUT) return;

  CopyProgressBox.ShowProgress(read, write, total, readTime, writeTime,
                                readN, writeN, totalN, Parallel, FirstWrite, 
                                StartTime, BufSize);
  SetEvent(UiFree);
}

int Engine::CheckOverwrite2(int fnum, const String& src, 
                            const String& dst, String& ren)
{
  WaitForSingleObject(UiFree, INFINITE);
  int res=CheckOverwrite(fnum, src, dst, ren);
  SetEvent(UiFree);
  return res;
}

void Engine::Delay(__int64 time, __int64 cb, __int64& counter,
                   __int64 limit)
{
  if (limit<=0) return;
  __int64 wait=(__int64)((double)cb/limit*TicksPerSec())-time;
  if (wait>0) 
  {
    Sleep((int)((double)wait/TicksPerSec()*1000));
    counter+=wait;
  }
}

