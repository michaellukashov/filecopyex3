#include <stdhdr.h>
#include "far/farplugin.h"
#include "far/farpanel.h"

FarPanel::FarPanel()
{
  Flags=OPIF_USEFILTER | OPIF_USESORTGROUPS | OPIF_USEHIGHLIGHTING |
    OPIF_REALNAMES | OPIF_SHOWNAMESONLY;
  CurDir="";
  FormatName="someplug";
  PanelTitle="Some plugin";
  Rescan=1;
}

FarPanel::~FarPanel()
{
}

void FarPanel::CallGetOpenPluginInfo(OpenPluginInfo *info)
{
  info->StructSize=sizeof(*info);
  info->Flags=Flags;
  info->HostFile=NULL;
  (FormatName+":\\"+CurDir).ToOem(_CurDirBuf, MAX_FILENAME);
  info->CurDir=_CurDirBuf;
  FormatName.ToOem(_FormatBuf, sizeof(_FormatBuf)/sizeof(TCHAR));
  PanelTitle.ToOem(_TitleBuf, sizeof(_TitleBuf)/sizeof(TCHAR));
  info->Format=_FormatBuf;
  info->PanelTitle=_TitleBuf;
}

int FarPanel::CallSetDirectory(const String& _dir, int opmode)
{
  String dir=_dir, NewDir=CurDir;
  if (dir=="..") 
  {
    if (NewDir=="") 
    {
      if (Flags & OPIF_ADDDOTS) 
        Info.Control(this, FCTL_CLOSEPLUGIN, NULL);
      else 
        return FALSE;
    }
    int p=NewDir.crfind('\\');
    if (p==-1) NewDir="";
    NewDir=NewDir.substr(0, p);
  }
  else if (dir.left(FormatName.len()+2)==FormatName+":\\") 
    NewDir=NewDir.substr(FormatName.len()+2);
  else 
  {
    if (NewDir!="") NewDir+="\\";
    NewDir+=dir;
  }
  int res=ChangeDir(dir, opmode & OPM_FIND, NewDir);
  if (res) Rescan=1;
  return res;
}

int FarPanel::CallGetFindData(PluginPanelItem* &items, int &nitems, int opmode)
{
  if (Rescan)
  {
    Files.Clear();
    if (!ReadFileList(opmode & OPM_FIND)) return FALSE;
    Rescan=0;
  }
  items=(PluginPanelItem*)Files.Storage();
  nitems=Files.Count();
  return TRUE;
}

void FarPanel::CallFreeFindData(PluginPanelItem *items, int nitems)
{
  Files.Clear();
  Rescan=1;
}

int FarPanel::CallDeleteFiles(PluginPanelItem *items, int nitems, int opmode)
{
  int res=DelFiles(items, nitems, opmode & OPM_SILENT);
  Rescan=1;
  return res;
}

int FarPanel::CallPutFiles(PluginPanelItem *items, int nitems, int move, int opmode)
{
  int res=PutFiles(items, nitems, move, opmode & OPM_SILENT);
  Rescan=1;
  return res;
}

int FarPanel::CallGetFiles(PluginPanelItem *items, int nitems, int move, 
                           const String& dest, int opmode)
{
  int res=GetFiles(items, nitems, move, dest, opmode & OPM_SILENT);
  Rescan=1;
  return res;
}

int FarPanel::CallMakeDirectory(String &name, int opmode)
{
  int res=MkDir(name, opmode & OPM_SILENT);
  Rescan=1;
  return res;
}

int FarPanel::ChangeDir(String& dir, int silent, const String& suggest)
{
  CurDir=suggest;
  return TRUE;
}

int FarPanel::ReadFileList(int silent)
{
  return TRUE;
}

int FarPanel::DelFiles(PluginPanelItem* files, int count, int silent)
{
  return FALSE;
}

int FarPanel::PutFiles(PluginPanelItem* files, int count, int move, int silent)
{
  return FALSE;
}

int FarPanel::GetFiles(PluginPanelItem* files, int count, int move, 
                       const String& dest, int silent)
{
  return FALSE;
}

int FarPanel::MkDir(String &name, int silent)
{
  return FALSE;
}

void _export WINAPI GetOpenPluginInfo(HANDLE hPlugin, struct OpenPluginInfo *Info)
{
  SetFileApisToANSI();
  ((FarPanel*)hPlugin)->CallGetOpenPluginInfo(Info);
  SetFileApisToOEM();
}

int _export WINAPI SetDirectory(HANDLE hPlugin, const char *Dir, int OpMode)
{
  SetFileApisToANSI();
  char *dir=(char*)_alloca(strlen(Dir)+1);
  strcpy(dir, Dir);
  _toansi(dir);
  int res=((FarPanel*)hPlugin)->CallSetDirectory(dir, OpMode);
  SetFileApisToOEM();
  return res;
}

int _export WINAPI GetFindData(HANDLE hPlugin,  struct PluginPanelItem **pPanelItem,
                               int *pItemsNumber, int OpMode)
{
  SetFileApisToANSI();
  int res=((FarPanel*)hPlugin)->CallGetFindData(*pPanelItem, *pItemsNumber, OpMode);
  SetFileApisToOEM();
  return res;
}

void _export WINAPI FreeFindData(HANDLE hPlugin, struct PluginPanelItem *PanelItem,
                                 int ItemsNumber)
{
  SetFileApisToANSI();
  ((FarPanel*)hPlugin)->CallFreeFindData(PanelItem, ItemsNumber);
  SetFileApisToOEM();
}

int _export WINAPI DeleteFiles(HANDLE hPlugin, struct PluginPanelItem *PanelItem,
                               int ItemsNumber, int OpMode)
{
  SetFileApisToANSI();
  int res=((FarPanel*)hPlugin)->CallDeleteFiles(PanelItem, ItemsNumber, OpMode);
  SetFileApisToOEM();
  return res;
}

int _export WINAPI PutFiles(HANDLE hPlugin, struct PluginPanelItem *PanelItem,
                               int ItemsNumber, int Move, int OpMode)
{
  SetFileApisToANSI();
  int res=((FarPanel*)hPlugin)->CallPutFiles(PanelItem, ItemsNumber, Move, OpMode);
  SetFileApisToOEM();
  return res;
}

int _export WINAPI GetFiles(HANDLE hPlugin, struct PluginPanelItem *PanelItem,
                            int ItemsNumber, int Move, char *DestPath,
                            int OpMode)
{
  SetFileApisToANSI();
  char *dp=(char*)_alloca(strlen(DestPath)+1);
  strcpy(dp, DestPath);
  _toansi(dp);
  int res=((FarPanel*)hPlugin)->CallGetFiles(PanelItem, ItemsNumber, Move, 
    dp, OpMode);
  SetFileApisToOEM();
  return res;
}

int _export WINAPI MakeDirectory(HANDLE hPlugin, char *Name, int OpMode)
{
  SetFileApisToANSI();
  char *dp=(char*)_alloca(strlen(Name)+1);
  strcpy(dp, Name);
  _toansi(dp);
  String name=dp;
  int res=((FarPanel*)hPlugin)->CallMakeDirectory(name, OpMode);
  if (res==TRUE && name!=Name) name.ToOem(Name, MAX_PATH);
  SetFileApisToOEM();
  return res;

}
