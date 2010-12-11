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

#include "../framework/stdhdr.h"
#include "../framework/lowlevelstr.h"
#include "common.h"
#include "engine.h"
#include "../framework/far/plugin.h"
#include "filecopyex.h"
#include "../framework/strutils.h"
#include "../framework/fileutils.h"
#include "EngineTools.h"
#include "../framework/common.h"

#define EXCEPTION_ABORT_BY_ESC  0x0F


int warn(const String& s)
{
  return ShowMessage(LOC("CopyDialog.Warning"), s+"\n"+LOC("CopyDialog.WarnPrompt"), FMSG_MB_YESNO) == 0;
}


void ShowErrorMessage(const String& s)
{
  //ShowMessage(LOC("CopyDialog.Error"), s, FMSG_WARNING | FMSG_MB_OK);

  String msgbuf = LOC("CopyDialog.Error") + "\n" + s + "\n";

  Info.Message(Info.ModuleNumber, 
                FMSG_WARNING | FMSG_MB_OK | FMSG_ALLINONE, 
                NULL, 
                (const wchar_t**)(const wchar_t*)msgbuf.ptr(), 0, 0);
}


__int64 GetPhysMemorySize()
{
  MEMORYSTATUS ms;
  GlobalMemoryStatus(&ms);
  return ms.dwTotalPhys;
}

struct TPanelItem
{
public:
	TPanelItem(int idx, bool active = true, bool selected = false)
	{
		ppi = (PluginPanelItem*)malloc(Info.Control(active ? PANEL_ACTIVE : PANEL_PASSIVE, selected ? FCTL_GETSELECTEDPANELITEM : FCTL_GETPANELITEM, idx, NULL));
		if(ppi)
			Info.Control(active ? PANEL_ACTIVE : PANEL_PASSIVE, selected ? FCTL_GETSELECTEDPANELITEM : FCTL_GETPANELITEM, idx, (LONG_PTR)ppi);
	}
	~TPanelItem()
	{
		free(ppi);
	}
	PluginPanelItem* operator->() const { return ppi; }
protected:
	PluginPanelItem* ppi;
};

void AddToQueue(int curonly)
{
	//  SetFileApisToOEM();
	PanelInfo pi;
	Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (LONG_PTR)&pi);
	// fixed by Nsky: bug #40
	//  SetFileApisToANSI();
	if(pi.PanelType==PTYPE_QVIEWPANEL || pi.PanelType== PTYPE_INFOPANEL ||
		!pi.ItemsNumber) return;

	Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (LONG_PTR)&pi);
	wchar_t cur_dir[MAX_FILENAME];
	Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, MAX_FILENAME, (LONG_PTR)cur_dir);
	if(pi.Plugin && wcsncmp(cur_dir, L"queue:", 6)) 
	{
		ShowMessage(LOC("PluginName"), LOC("CopyDialog.CantAddToQueue"), FMSG_MB_OK);
		return;
	}

	String srcpath=CutEndSlash(cur_dir);
	if(srcpath.left(7)=="queue:\\") srcpath=srcpath.substr(7);
	else if(srcpath=="queue:") srcpath="";

	int s = curonly?pi.CurrentItem:0,
		e = curonly?pi.CurrentItem:pi.SelectedItemsNumber-1;
//	PluginPanelItem* data = curonly?pi.PanelItems:pi.SelectedItems;
	for (int i = s; i <= e ; i++)
	{
		TPanelItem pit(i, pi.Focus == TRUE, !curonly);
		if(!wcscmp(pit->FindData.lpwszFileName, L"..")) 
			continue;

		String file;
		if(pit->FindData.lpwszFileName[0])
		{
			file = pit->FindData.lpwszFileName;
			if(file.cfind('\\')==-1)
				file=AddEndSlash(srcpath)+file;
		}
		if(file=="" || !FileHasUnicodeName(file))
		{
			file = pit->FindData.lpwszFileName;
			if(file.cfind('\\')==-1)
				file=AddEndSlash(srcpath)+file;
		}

		if(TempFiles.Find(file)==-1)
			TempFiles.Add(file);
	}

	SaveTemp();
	SetFileApisToOEM();
	Info.Control(PANEL_PASSIVE, FCTL_UPDATEPANEL, 1, NULL);
	Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, NULL);
//	fixed by Nsky: bug #40
//	SetFileApisToANSI();
}
/*
static PluginPanelItem* SortData;

int MyCompare(const int &i, const int &j, const void*)
{
  char *p1=strrchr(SortData[i].FindData.cFileName, '\\'),
       *p2=strrchr(SortData[j].FindData.cFileName, '\\');
  if(!p1 && !p2) return 0;
  else if(!p1) return -1;
  else if(!p2) return 1;
  else
  {
    char t1=*p1, t2=*p2;
    *p1=*p2=0;
    int res=_stricmp(SortData[i].FindData.cFileName,
      SortData[j].FindData.cFileName);
    *p1=t1; *p2=t2;
    return res;
  }
}
*/
String Engine::FindDescFile(const String& dir, int *idx)
{
  for (int i=0; i<DescFiles.Count(); i++)
    if(FileExists(AddEndSlash(dir)+DescFiles[i]))
    {
      if(idx) *idx=i;
      return DescFiles[i];
    }
  if(idx) *idx=-1;
  return "";
}

String Engine::FindDescFile(const String& dir, WIN32_FIND_DATA &fd, int *idx)
{
  HANDLE hf;
  for (int i=0; i<DescFiles.Count(); i++)
    if((hf=FindFirstFile((AddEndSlash(dir)+DescFiles[i]).ptr(), &fd)) 
      != INVALID_HANDLE_VALUE)
    {
      FindClose(hf);
      if(idx) *idx=i;
      return DescFiles[i];
    }
  if(idx) *idx=-1;
  return "";
}

int Engine::AddFile(const String& Src, const String& Dst, 
                    WIN32_FIND_DATA &fd, int Flags, int Level, int PanelIndex)
{
  return AddFile(Src, Dst, fd.dwFileAttributes, MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh),
                                    fd.ftLastWriteTime, Flags, Level, PanelIndex);
}

void Engine::AddTopLevelDir(const String &dir, const String &dstmask, 
                            int Flags, wchar_t pc)
{
  HANDLE hf;
  WIN32_FIND_DATA fd;

  SrcNames.AddRel(pc, dir.ptr());
  DstNames.AddRel(pc, ExtractFilePath(
    ApplyFileMaskPath(dir+"\\somefile.txt", dstmask)).ptr());
  FileStruct _info;
  memset(&_info, 0, sizeof(_info));
  FileStruct &info=Files[Files.Add(_info)];
  info.Flags=Flags;
  info.Level=0;
  info.PanelIndex=-1;
  if((hf=FindFirstFile(dir.ptr(), &fd))
    !=INVALID_HANDLE_VALUE)
  {
    FindClose(hf);
    info.Attr=fd.dwFileAttributes;
    info.Modify=fd.ftLastWriteTime;
  }
}

int Engine::DirStart(const String& dir, const String& dstmask)
{
  if(_CopyDescs)
    CurPathDesc=FindDescFile(dir, DescFindData);
  CurPathFlags=CurPathAddFlags=0;
  if(_ClearROFromCD && VolFlags(dir) & VF_CDROM) 
    CurPathAddFlags|=AF_CLEAR_RO;
  AddTopLevelDir(dir, dstmask, 
    FLG_DIR_PRE | FLG_DIR_FORCE | FLG_TOP_DIR | CurPathFlags, '+');
  return TRUE;
}

int Engine::DirEnd(const String& dir, const String& dstmask)
{
  if(_CopyDescs && CurPathDesc != "")
  {
    if(!AddFile(dir+"\\"+CurPathDesc, 
      AddEndSlash(ExtractFilePath(
        ApplyFileMaskPath(dir+"\\"+CurPathDesc, dstmask)))+CurPathDesc,
      DescFindData, AF_DESCFILE | AF_DESC_INVERSE | CurPathAddFlags, 1))
        return FALSE;
  }
  AddTopLevelDir(dir, dstmask, 
    FLG_DIR_POST | FLG_DIR_NOREMOVE | CurPathFlags, '*');
  return TRUE;
}

void Engine::FarToWin32FindData(const FAR_FIND_DATA &fd, WIN32_FIND_DATA &wfd)
{
  wfd.dwFileAttributes=fd.dwFileAttributes;
  wfd.ftCreationTime=fd.ftCreationTime;
  wfd.ftLastAccessTime=fd.ftLastAccessTime;
  wfd.ftLastWriteTime=fd.ftLastWriteTime;
  LARGE_INTEGER x;
  x.QuadPart = fd.nFileSize;
  wfd.nFileSizeHigh=x.HighPart;
  wfd.nFileSizeLow=x.LowPart;
}

int Engine::Main(int move, int curonly)
{
  PropertyList& Options = Instance->Options;

  _CopyDescs    = Options["CopyDescs"];
  _DescsInDirs  = Options["DescsInSubdirs"];
  _ConfirmBreak = Options["ConfirmBreak"];
  _PreallocMin  = Options["PreallocMin"];
  _UnbuffMin    = Options["UnbuffMin"];

  _ClearROFromCD  = Info.AdvControl(Info.ModuleNumber, ACTL_GETSYSTEMSETTINGS, NULL) & FSS_CLEARROATTRIBUTE;
  _HideDescs      = Info.AdvControl(Info.ModuleNumber, ACTL_GETDESCSETTINGS, NULL) & FDS_SETHIDDEN;
  _UpdateRODescs  = Info.AdvControl(Info.ModuleNumber, ACTL_GETDESCSETTINGS, NULL) & FDS_UPDATEREADONLY;

  FarDialog& dlg = Instance->Dialogs["CopyDialog"];
  dlg.ResetControls();
  FarDialog& advdlg = Instance->Dialogs["AdvCopyDialog"];
  advdlg.ResetControls();

  PanelInfo pi;
  String prompt, srcpath, dstpath;
  Move = move;
  int allowPlug=0, adv=0;

  wchar_t cur_dir[MAX_FILENAME];
  Info.Control(PANEL_PASSIVE, FCTL_GETPANELDIR, MAX_FILENAME, (LONG_PTR)cur_dir);
//  SetFileApisToOEM();
  Info.Control(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)&pi);
  // fixed by Nsky: bug #40
//  SetFileApisToANSI();
//  _toansi(pi.CurDir);

  if(pi.Plugin)
  {
    if(!wcsncmp(cur_dir, L"queue:\\", 7)) 
    {
      if(!wcscmp(cur_dir, L"queue:\\")) dstpath="queue:"; 
      else dstpath=Replace(String(cur_dir).substr(7), "/", "\\")+"\\";
    }
    else dstpath="plugin:";
    allowPlug=1;
  }
  else 
    if(pi.PanelType==PTYPE_QVIEWPANEL || pi.PanelType==PTYPE_INFOPANEL ||  !pi.Visible)
      dstpath="";
    else
      dstpath=AddEndSlash(cur_dir);

  Info.Control(PANEL_ACTIVE, FCTL_GETPANELDIR, MAX_FILENAME, (LONG_PTR)cur_dir);
//  SetFileApisToOEM();
  Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (LONG_PTR)&pi);
  // fixed by Nsky: bug #40
//  SetFileApisToANSI();
//  _toansi(pi.CurDir);
  if(pi.PanelType==PTYPE_QVIEWPANEL || pi.PanelType== PTYPE_INFOPANEL ||
     !pi.ItemsNumber) return MRES_NONE;
  // Bug #5 fixed by CDK
  if((pi.Flags & PFLAGS_REALNAMES) == 0) return MRES_STDCOPY;
  if(pi.SelectedItemsNumber>1 && !curonly)
  {
    if(!move) prompt=LOC("CopyDialog.CopyFilesTo");
    else 
    prompt=LOC("CopyDialog.MoveFilesTo");
  }
  else
  {
    wchar_t buf[MAX_FILENAME];
	bool pit_sel = pi.SelectedItemsNumber && !curonly;
	TPanelItem pit(pit_sel ? 0 : pi.CurrentItem, true, pit_sel);
    FAR_FIND_DATA &fd = pit->FindData;
    wcsncpy_s(buf, MAX_FILENAME, fd.lpwszFileName, MAX_FILENAME);
//    _toansi(buf);
    String fn=ExtractFileName(buf);
    if(fn=="..") return MRES_NONE;
    String fmt;
    if(!move) fmt=LOC("CopyDialog.CopyTo");
    else 
	    fmt=LOC("CopyDialog.MoveTo");
    prompt = Format(fmt.ptr(), fn.ptr());
    if(curonly || dstpath=="") dstpath=fn;
    /*else if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      && dstpath!="queue:" && dstpath!="plugin:")
        dstpath=AddEndSlash(dstpath)+fn;*/
  } 
  srcpath=CutEndSlash(cur_dir);
  if(srcpath.left(7)=="queue:\\") srcpath=srcpath.substr(7);
  else if(srcpath=="queue:") srcpath="";

  _InverseBars=(bool)Options["ConnectLikeBars"] && pi.PanelRect.left>0;
  
  if(!move) dlg("Title")=LOC("CopyDialog.Copy");
  else 
  dlg("Title")=LOC("CopyDialog.Move");
  dlg["Label1"]("Text") = prompt;
  dlg["DestPath"]("Text") = dstpath;

  // axxie: Reset cp in case when user pressed Shift-F6/Shift-F5
  int cp = (!curonly && srcpath!="") ? CheckParallel(srcpath, dstpath) : FALSE;
  if(!Options["AllowParallel"]) 
    dlg["ParallelCopy"]("Selected")=FALSE;
  else 
  {
    if(cp==1) dlg["ParallelCopy"]("Selected")=TRUE;
    else 
      if(cp==0) dlg["ParallelCopy"]("Selected")=FALSE;
      else 
        dlg["ParallelCopy"]("Selected")=Options["DefParallel"];
  }
  dlg["Ask"]("Selected") = !Options["OverwriteDef"];
  dlg["Overwrite"]("Selected") = Options["OverwriteDef"];
  
  if(WinNT)
  {
    advdlg["Streams"]("Selected") = Options["CopyStreamsDef"];
    advdlg["Rights"]("Selected") = Options["CopyRightsDef"];
  }
  else
  {
    advdlg["Streams"]("Disable") = 1;
    advdlg["Rights"]("Disable") = 1;
    advdlg["Compress"]("Disable") = 1;
  }
  if(!Win2K) advdlg["Encrypt"]("Disable") = 1;

rep:

  int dres=dlg.Execute();
  if(dres==2) 
  {
    ((FileCopyExPlugin*)Instance)->Config();
    goto rep;
  }
  else if(dres==1)
  {
    if(advdlg.Execute()==0)
    {
      adv=1;
      bool resume=advdlg["ResumeFiles"]("Selected");
      dlg["Label2"]("Disable")=resume;
      dlg["Ask"]("Disable")=resume;
      dlg["Skip"]("Disable")=resume;
      dlg["Overwrite"]("Disable")=resume;
      dlg["Append"]("Disable")=resume;
      dlg["Rename"]("Disable")=resume;
      dlg["SkipIfNewer"]("Disable")=resume;
      if(resume)
        dlg["SkipIfNewer"]("Selected")=0;
    }
    goto rep;
  }
  else if(dres==-1) return MRES_NONE;

  String tmpdsttext=dlg["DestPath"]("Text");

  // bugfixed by slst: 
  String dsttext = tmpdsttext.trim().trimquotes();

  // fixed by Nsky: bug #28
  if(!dsttext.left(4).icmp("nul\\"))
    dsttext = _T("nul");

  // bugfixed by slst: bug #7     
  dsttext = Replace(dsttext, "\"", "");

  if(dsttext=="plugin:") 
  {
    if(allowPlug) return MRES_STDCOPY_RET;
    else 
    {
      ShowMessage(dlg("Text"), LOC("CopyDialog.InvalidPath"), FMSG_MB_OK);
      goto rep;
    }
  }
  else if(dsttext=="queue:")
  {
    AddToQueue(curonly);
    return MRES_NONE;
  }

  String RelDstPath = ExpandEnv(Replace(dsttext, "/", "\\"));
  dstpath = "";
  
  wchar_t CurrentDir[MAX_FILENAME];
  wchar_t dstbuf[MAX_FILENAME];
  wchar_t *FNCPtr;
  memset(dstbuf, 0, sizeof(dstbuf));
  // Get absolute path for relative dstpath
  GetCurrentDirectory(MAX_FILENAME, CurrentDir);
  // srcpath.ptr() for temporary file panel is empty 
  // Current directory is set by Far to file path of selected file
  BOOL SCDResult = SetCurrentDirectory(srcpath.ptr());
  // fixed by Nsky: bug #28
  if(RelDstPath.icmp("nul")) {
    if(GetFullPathName(RelDstPath.ptr(), MAX_FILENAME, dstbuf, &FNCPtr)) dstpath = dstbuf;
  } else {
    dstpath=RelDstPath;
  }
  if(SCDResult) SetCurrentDirectory(CurrentDir);
  
  CompressMode=EncryptMode=ATTR_INHERIT;
  Streams=Rights=FALSE;

  if(WinNT)
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

  Parallel=dlg["ParallelCopy"]("Selected");
  SkipNewer=dlg["SkipIfNewer"]("Selected");
  SkippedToTemp=advdlg["SkippedToTemp"]("Selected");
  ReadSpeedLimit=WriteSpeedLimit=0;
  if((bool)advdlg["ReadSpeedLimit"]("Selected"))
    ReadSpeedLimit=(int)advdlg["ReadSpeedLimitValue"]("Text")*1024;
  if((bool)advdlg["WriteSpeedLimit"]("Selected"))
    WriteSpeedLimit=(int)advdlg["WriteSpeedLimitValue"]("Text")*1024;

  if(advdlg["ResumeFiles"]("Selected")) OverwriteMode=OM_RESUME;
  else if(dlg["Overwrite"]("Selected")) OverwriteMode=OM_OVERWRITE;
  else if(dlg["Skip"]("Selected")) OverwriteMode=OM_SKIP;
  else if(dlg["Append"]("Selected")) OverwriteMode=OM_APPEND;
  else if(dlg["Rename"]("Selected")) OverwriteMode=OM_RENAME;
  else OverwriteMode=OM_PROMPT;

  // fixed by Nsky: bug #28
  if(!dstpath.icmp("nul"))
  {
    OverwriteMode=OM_OVERWRITE;
    CompressMode=EncryptMode=ATTR_INHERIT;
    Streams=Rights=0;
  }
  else
  {
    int vf=VolFlags(dstpath);
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
  if(GetRealFileName(srcpath) == GetRealFileName(dstpath))
  {
    ShowErrorMessage(LOC("CopyDialog.OntoItselfError"));
    return MRES_NONE;
  }
  
  TotalBytes=0;
  TotalN=0;
  FileCount=CopyCount=0;
  __int64 Start=GetTime();

  // bugfixed by slst: bug #24
  //progress.ShowMessage(LOC("Status.CreatingList"));
  ScanFoldersProgressBox.ShowScanProgress(LOC("Status.ScanningFolders"));

  int s, e, curitem=curonly;
  if(!curitem)
  {
	curitem = !(TPanelItem(0, true, true)->Flags & PPIF_SELECTED);
  }
  if(curitem)
    s=e=pi.CurrentItem;
  else 
  {
    s=0;
    e=pi.ItemsNumber-1;
  }
//  PluginPanelItem *data = pi.PanelItems;
  int c=0;
  for (int i=s; i<=e; i++) 
  {
    if(curitem)
		++c;
	else
	{
		if(TPanelItem(i)->Flags & PPIF_SELECTED)
			++c;
	}
  }
  Array<int> SortIndex;
  SortIndex.Resize(c);
  c=0;
  for (int i=s; i<=e; i++)
  {
    if(curitem)
		SortIndex[c++]=i;
	else
	{
		if(TPanelItem(i)->Flags & PPIF_SELECTED)
			SortIndex[c++]=i;
	}
  }
//  SortData=data;
  //SortIndex.SetSorted(1, MyCompare);

  String curpath;
  int havecurpath=0;

  // bugfixed by slst: bug #23
  // Engine::Addfile throws exception with EXCEPTION_ABORT_BY_ESC parameter
  try
  {
    for (int ii=0; ii<c; ii++)
    {
      int i=SortIndex[ii];
	  TPanelItem pit(i);
      if(!wcscmp(pit->FindData.lpwszFileName, L"..")) 
        continue;

      wchar_t nbuf[MAX_FILENAME];
      if(pit->FindData.lpwszAlternateFileName[0]
      && !wcschr(pit->FindData.lpwszAlternateFileName, '\\'))
        wcsncpy_s(nbuf, MAX_FILENAME, pit->FindData.lpwszAlternateFileName, MAX_FILENAME);
      else
        wcsncpy_s(nbuf, MAX_FILENAME, pit->FindData.lpwszFileName, MAX_FILENAME);
//      _toansi(nbuf);
      String file=nbuf;

      String filepath=ExtractFilePath(file);
      if(!havecurpath || filepath.icmp(curpath))
      {
        if(havecurpath && 
          !DirEnd(curpath!=""?curpath:srcpath, dstpath)) goto fin;
        curpath=filepath;
        havecurpath=1;
        if(!DirStart(curpath!=""?curpath:srcpath, dstpath)) goto fin;
      }

      if(file.cfind('\\')==-1) file=AddEndSlash(srcpath)+file;
      file=MyGetLongPathName(file);

      if(_CopyDescs && !ExtractFileName(file).icmp(CurPathDesc))
      {
        if(ShowMessageEx(LOC("CopyDialog.Warning"), 
          FormatWidthNoExt(file, 50)+"\n"+
          LOC("CopyDialog.DescSelectedWarn")+"\n"+
          LOC("CopyDialog.DescSelectedWarn1"), 
          LOC("Framework.No")+"\n"+LOC("Framework.Yes"), 0)!=1)
            continue;
        else
          CurPathDesc="";
      }

      String dst=ApplyFileMaskPath(file, dstpath);
      FAR_FIND_DATA *fd = &pit->FindData;
      WIN32_FIND_DATA wfd;
      FarToWin32FindData(*fd, wfd);
      if(!AddFile(file, dst, wfd, CurPathAddFlags, 1, i)) 
        goto fin;
    } // for (int ii=0; ii<c; ii++)
  }
  catch (int e) // bugfixed by slst: bug #23
  {
    if(e == EXCEPTION_ABORT_BY_ESC)
    {
      ScanFoldersProgressBox.Hide();
//      SetFileApisToOEM();
      Info.Control(PANEL_ACTIVE, FCTL_UPDATEPANEL, 1, NULL);
      Info.Control(PANEL_PASSIVE, FCTL_UPDATEPANEL, 1, NULL);
      Info.Control(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, NULL);
      Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, NULL);
      // fixed by Nsky: bug #40
//      SetFileApisToANSI();
      return MRES_NONE;
    }
  }
  // bugfixed by slst: bug #23 - end of fix

  if(havecurpath && !DirEnd(curpath!=""?curpath:srcpath, dstpath)) goto fin;

  if(OverwriteMode == OM_RESUME)
  {
    FileNameStoreEnum Enum(DstNames);
    // bugfixed by slst: bug #24
    //progress.ShowMessage(LOC("Status.ScanningDest"));
    ScanFoldersProgressBox.ShowScanProgress(LOC("Status.ScanningFolders"));
    for (int i=0; i<Enum.Count(); i++)
    {
      if(!(Files[i].Flags & FLG_SKIPPED) &
        !(Files[i].Attr & FILE_ATTRIBUTE_DIRECTORY))
      {
        __int64 sz=FileSize((String)Enum.GetByNum(i));
        if(sz<Files[i].Size)
        {
          Files[i].ResumePos=sz/ReadAlign*ReadAlign;
          TotalBytes-=Files[i].ResumePos;
        }
        else
        {
          Files[i].Flags|= FLG_SKIPPED;
          TotalBytes-=Files[i].Size;
          TotalN--;
        }
      }
    }
  }
  else if(OverwriteMode == OM_SKIP || OverwriteMode == OM_RENAME || SkipNewer)
  {
    // bugfixed by slst: bug #24
    //progress.ShowMessage(LOC("Status.ScanningDest"));
    ScanFoldersProgressBox.ShowScanProgress(LOC("Status.ScanningFolders"));
    SetOverwriteMode(0);
  }

  ScanFoldersProgressBox.Hide();

  if(Options["BufPercent"])
    BufSize=(int)(GetPhysMemorySize()/100*(int)Options["BufPercentVal"]);
  else
    BufSize=(int)Options["BufSizeVal"]*1024;

  // bugfixed by slst: bug #32
  // CheckFreeDiskSpace feature added
  if(Instance->Options["CheckFreeDiskSpace"])
  {
    if(!CheckFreeDiskSpace(TotalBytes, Move, srcpath, dstpath))
      return MRES_NONE; // not enough space
  }
      
  if(CopyCount) Copy();
  
  if((bool)Options["Sound"] && !Aborted)
  {
    if(GetTime()-Start > 30*60*TicksPerSec()) beep3();
    else if(GetTime()-Start > 10*60*TicksPerSec()) beep2();
    else if(GetTime()-Start > 30*TicksPerSec()) beep();
  }

fin:
  ScanFoldersProgressBox.Hide();

//  SetFileApisToOEM();

  if(!Move)
  {
    int i=0;
	Info.Control(PANEL_ACTIVE,FCTL_BEGINSELECTION,0,NULL);
    while (i<Files.Count())
    {
      if(Files[i].PanelIndex!=-1)
      {
        if(Files[i].Flags & FLG_DIR_PRE)
        {
          int ok=1, j=i+1;
          while (Files[j].Level>Files[i].Level)
          {
            if(!(Files[j].Flags & (FLG_DIR_PRE | FLG_DIR_POST | FLG_COPIED)))
            {
              ok=0; break;
            }
            j++;
          }
          if(ok)
		  {
			Info.Control(PANEL_ACTIVE, FCTL_SETSELECTION, Files[i].PanelIndex, FALSE);
		  }
          i=j-1;
        }
        else if(!(Files[i].Flags & FLG_DIR_POST) &&
          Files[i].Flags & FLG_COPIED)
		{
			Info.Control(PANEL_ACTIVE, FCTL_SETSELECTION, Files[i].PanelIndex, FALSE);
		}
      }
      i++;
    }
	Info.Control(PANEL_ACTIVE,FCTL_ENDSELECTION,0,NULL);
    // fixed by Nsky: bug #40
//    SetFileApisToANSI();
  }
  else
  {
    // Bugfixed by slst: bug #2
    Info.Control(PANEL_ACTIVE, FCTL_UPDATEPANEL, 1, NULL);
    Info.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)&pi);
    // fixed by Nsky: bug #40
//    SetFileApisToANSI();
    PanelRedrawInfo rpi;
    rpi.TopPanelItem = pi.TopPanelItem;

    String NewFileName;
    for (int idx=0; idx<Files.Count(); idx++)
    {
      if(Files[idx].PanelIndex == pi.CurrentItem)
      {
        NewFileName = DstNames.GetNameByNum(idx);
        break;
      }
    }
    NewFileName = NewFileName.toLower();

    for (int i=0; i<pi.ItemsNumber; i++)
    {
		TPanelItem pit(i);
	  String NewPanelFilename = pit->FindData.lpwszFileName;
	  NewPanelFilename = NewPanelFilename.toLower();
	  if(NewFileName == NewPanelFilename)
	  {
		rpi.CurrentItem = i;
//        SetFileApisToOEM();
		Info.Control(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, (LONG_PTR)&rpi);
		// fixed by Nsky: bug #40
//        SetFileApisToANSI();
		break;
	  }
    }
  }

//  SetFileApisToOEM();
  Info.Control(PANEL_ACTIVE,	FCTL_UPDATEPANEL, 1, NULL);
  Info.Control(PANEL_PASSIVE,	FCTL_UPDATEPANEL, 1, NULL);
  Info.Control(PANEL_ACTIVE,	FCTL_REDRAWPANEL, 0, NULL);
  Info.Control(PANEL_PASSIVE,	FCTL_REDRAWPANEL, 0, NULL);
  // fixed by Nsky: bug #40
//  SetFileApisToANSI();

  return MRES_NONE;
}


int Engine::AddFile(const String& _src, const String& _dst, int attr, 
                     __int64 size, FILETIME& Modify, int flags, int Level,
                    int PanelIndex)
{
  // bugfixed by slst: bug #23
  if(CheckEscape(FALSE)) throw EXCEPTION_ABORT_BY_ESC;

  if(attr==0xFFFFFFFF) return TRUE;

  String src = _src;
  String dst = _dst;
    
  // bugfixed by slst:
  // Get here the real file names 
  // unfold symlinks in file paths (if any)
  // src path
//  DWORD _src_attr = GetFileAttributes(ExtractFilePath(_src).ptr());
//  if((_src_attr != 0xFFFFFFFF) && 
//          (_src_attr & FILE_ATTRIBUTE_DIRECTORY)  && 
//          (_src_attr & FILE_ATTRIBUTE_REPARSE_POINT))
//      src = AddEndSlash(GetRealFileName(ExtractFilePath(_src))) + ExtractFileName(_src);
//  else
//      src = _src;
//
//  // dst path
//  DWORD _dst_attr = GetFileAttributes(ExtractFilePath(_dst).ptr());
//  if((_dst_attr != 0xFFFFFFFF) && 
//      (_dst_attr & FILE_ATTRIBUTE_DIRECTORY)  && 
//      (_dst_attr & FILE_ATTRIBUTE_REPARSE_POINT))
//      dst = AddEndSlash(GetRealFileName(ExtractFilePath(_dst))) + ExtractFileName(_dst);
//  else
//      dst = _dst;
  //////////////////////////////////////////////////////
  
  if(src==dst && !(flags & AF_DESCFILE)) return TRUE;

  if(flags & AF_CLEAR_RO)
    attr &= ~FILE_ATTRIBUTE_READONLY;
  
  __int64 sz1;
  if(attr & FILE_ATTRIBUTE_DIRECTORY) sz1 = 0;
  else sz1 = size;

  FileStruct _info;
  memset(&_info, 0, sizeof(_info));
  FileStruct &info=Files[Files.Add(_info)];
  info.Size=sz1;
  info.Attr=attr;
  info.Modify=Modify;
  info.Level=Level;
  info.PanelIndex=PanelIndex;
  wchar_t pc;
  if(attr & FILE_ATTRIBUTE_DIRECTORY) 
  {
    info.Flags|=FLG_DIR_PRE;
    pc='+';
  }
  else pc=' ';

  SrcNames.AddRel(pc, ExtractFileName(src).ptr());
  DstNames.AddRel(pc, ExtractFileName(dst).ptr());

  if(flags & AF_DESCFILE) 
  {
    info.Flags |= FLG_DESCFILE;
    if(flags & AF_DESC_INVERSE) info.Flags |= FLG_DESC_INVERSE;
    CopyCount++;
    return TRUE;
  }

  int owmode = OM_PROMPT;
  if(!(flags & AF_STREAM))
  {
    if(Move)
    {
      retry:
      if(MoveFile(src, dst, FALSE)) 
      {
        info.Flags |= FLG_COPIED | FLG_DELETED;
        goto fin;
      }
      int err = GetLastError();
      if(err == ERROR_ALREADY_EXISTS && !(attr & FILE_ATTRIBUTE_DIRECTORY))
      {
        if(OverwriteMode == OM_RESUME)
        {
          if(FileSize(dst)>=info.Size)
          {
            info.Flags |= FLG_SKIPPED;
            goto fin;
          }
        }
        else
        {
          String ren;
          int res, j = 0;
          if(SkipNewer && Newer2(dst, Modify))
            res = OM_SKIP;
          else if(OverwriteMode == OM_PROMPT)
            res = CheckOverwrite(-1, src, dst, ren);
          else 
            res = OverwriteMode;
          switch (res)
          {
            case OM_SKIP: 
              info.Flags |= FLG_SKIPPED;
              goto fin;
            break;

            case OM_OVERWRITE: 
              owmode = OM_OVERWRITE;
              if(MoveFile(src, dst, TRUE)) 
              {
                info.Flags |= FLG_COPIED | FLG_DELETED;
                goto fin;
              }
            break;

            case OM_APPEND:
              owmode = OM_APPEND;
            break;

            case OM_RENAME:
              if(OverwriteMode != OM_RENAME)
                dst=ren;
              else
              {
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
  if(!(attr & FILE_ATTRIBUTE_DIRECTORY)) FileCount++; 
  TotalBytes += sz1;
  info.OverMode = owmode;

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
          if(_tcscmp(fd.cFileName, _T("..")) && _tcscmp(fd.cFileName, _T(".")))
          {
            int idx;
            if(_CopyDescs && _DescsInDirs 
              && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
              && (idx=DescFiles.Find(fd.cFileName))!=-1)
            {
              if(descidx=-1 || idx<descidx)
              {
                RememberFile(src+"\\"+fd.cFileName, dst+"\\"+fd.cFileName, 
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
            wchar_t strnb[MAX_FILENAME];
            _wtotcs(strnb, strn, MAX_FILENAME);
            if(!AddFile(src+strnb, dst+strnb, attr, 
              sid.Size.QuadPart, Modify, flags | AF_STREAM, Level+1))
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
  if(attr & FILE_ATTRIBUTE_DIRECTORY) 
  {
    FileStruct _info1;
    memset(&_info1, 0, sizeof(_info1));
    FileStruct &info1=Files[Files.Add(_info1)];
    info1.Flags|=FLG_DIR_POST | info.Flags & (FLG_COPIED | FLG_DELETED);
    info1.Level=Level;
    info1.PanelIndex=-1;
    SrcNames.AddRel('-', ExtractFileName(src).ptr());
    DstNames.AddRel('-', ExtractFileName(dst).ptr());
  }

  return TRUE;
}

void Engine::SetOverwriteMode(int Start)
{
  FileNameStoreEnum Enum(DstNames);
  for (int i=Start; i<Enum.Count(); i++)
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
    if(!(info.Flags & FLG_SKIPPED)     && 
        !(info.Flags & FLG_DIR_PRE)     && 
        !(info.Flags & FLG_DIR_POST)    && 
        FileExists(fn))
    {
      if(SkipNewer && Newer2(fn, info.Modify))
      {
        TotalBytes -= info.Size;
        TotalN--;
        info.Flags |= FLG_SKIPPED | FLG_SKIPNEWER;
      }
      else
        switch (OverwriteMode)
        {
        case OM_SKIP:
          TotalBytes -= info.Size;
          TotalN--;
          info.Flags |= FLG_SKIPPED;
          break;
        case OM_RENAME:
          {
            int j=0;
            while (ExistsN(fn, j)) j++;
            info.RenameNum = j;
          }
          break;
        }
    }
  }
}

int Engine::CheckOverwrite(int fnum, const String& Src, const String& Dst, String& ren)
{
  FarDialog &dlg=Instance->Dialogs["OverwriteDialog"];
  dlg.ResetControls();

  dlg["Label2"]("Text")=Dst;
  String ssz, dsz, stime, dtime, buf;

  WIN32_FIND_DATA fd, fs;
  FindClose(FindFirstFile(Dst.ptr(), &fd));
  dsz=FormatNum(MAKEINT64(fd.nFileSizeLow, fd.nFileSizeHigh));
  dtime=FormatTime(fd.ftLastWriteTime);
  dlg["Label4"]("Text")=Format(L"%14s %s  %s", dsz.ptr(), LOC("OverwriteDialog.Bytes").ptr(), dtime.ptr());

  FindClose(FindFirstFile(Src.ptr(), &fs));
  ssz=FormatNum(MAKEINT64(fs.nFileSizeLow, fs.nFileSizeHigh));
  stime=FormatTime(fs.ftLastWriteTime);
  dlg["Label3"]("Text")=Format(L"%14s %s  %s", ssz.ptr(), LOC("OverwriteDialog.Bytes").ptr(), stime.ptr());

  dlg["Source"]("Text")       = Format(L"%-16s", LOC("OverwriteDialog.Source").ptr());
  dlg["Destination"]("Text")  = Format(L"%-16s", LOC("OverwriteDialog.Destination").ptr());

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
    FarDialog &dlg=Instance->Dialogs["RenameDialog"];
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
  Remember.Modify=fd.ftLastWriteTime;
  Remember.Flags=Flags;
  Remember.Level=Level;
}

int Engine::AddRemembered(RememberStruct &Remember)
{
  return AddFile(Remember.Src, Remember.Dst, Remember.Attr,
    Remember.Size, Remember.Modify, Remember.Flags, Remember.Level);
}


// bugfixed by slst: bug #32
// Returns TRUE if there is enough space on target disk
BOOL Engine::CheckFreeDiskSpace(const __int64 TotalBytesToProcess, const int MoveMode,
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
  
      FarDialog &dlg=Instance->Dialogs["FreeSpaceErrorDialog"];
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
