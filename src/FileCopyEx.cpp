#include "../framework/stdhdr.h"
#include "common.h"
#include "engine.h"
#include "filecopyex.h"
#include "temppanel.h"
#include "../framework/far/interface/farkeys.hpp"

StringList DescFiles;

FileCopyExPlugin::FileCopyExPlugin(void)
{
  RootKey="FileCopyEx";
  MenuItems.Add("[&>] Extended copy");
  ConfigItems.Add("Extended copy");
  MiscInit();
  Registry.ReadList(String("\\")+Info.RootKey+"\\FileCopyEx\\Queue", TempFiles);
}

void SaveTemp()
{
  Registry.WriteList("Queue", TempFiles);
}

FileCopyExPlugin::~FileCopyExPlugin(void)
{
}

int getmod();

int FileCopyExPlugin::Configure(int)
{
  Config();
  return TRUE;
}

void CallCopy(int move, int curonly)
{
  Engine engine;
  int res=engine.Main(move, curonly);
  if (res==MRES_STDCOPY || res==MRES_STDCOPY_RET)
  {
    KeySequence seq;
    DWORD keys[8];
    seq.Flags=KSFLAGS_DISABLEOUTPUT;
    seq.Sequence=keys;
    seq.Count=1;
    keys[0]=move ? KEY_F6 : KEY_F5;
    if (res==MRES_STDCOPY_RET)
    {
      seq.Count=2;
      keys[1]=KEY_ENTER;
    }
    Info.AdvControl(Info.ModuleNumber, ACTL_POSTKEYSEQUENCE, (void*)&seq);
  }
}

void AddToQueue(int curonly);

FarPanel* FileCopyExPlugin::OpenPlugin(int from, int item)
{
  int move, curonly;

  FarMenu menu;
  menu.SetFlags(FMENU_WRAPMODE);
  menu.SetTitle(LOC("PluginName"));
  menu.SetBottom(LOC("PluginCopyright"));
  menu.SetHelpTopic("Menu");
  menu.AddLine(LOC("Menu.Item1"));
  menu.AddLine(LOC("Menu.Item2"));
  menu.AddLine(LOC("Menu.Item3"));
  menu.AddLine(LOC("Menu.Item4"));
  menu.AddSep();
  menu.AddLine(LOC("Menu.AddToQueue"));
  menu.AddLine(LOC("Menu.ShowQueue"));
  menu.AddSep();
  menu.AddLine(LOC("Menu.Config"));
  
  switch (menu.Execute())
  {
    case 0: move = 0; curonly = 0; break;
    case 1: move = 1; curonly = 0; break;
    case 2: move = 0; curonly = 1; break;
    case 3: move = 1; curonly = 1; break;
    case 5: AddToQueue(0); return NULL;
    case 6: return new TempPanel();
    case 8: Config(); return NULL; 
    default: return NULL;
  }

  CallCopy(move, curonly);

  return NULL;
}

void FileCopyExPlugin::InitOptions(PropertyList& options)
{
  options.Add("BufPercent", 1);
  options.Add("BufSize", 0);
  options.Add("BufPercentVal", 15);
  options.Add("BufSizeVal", 4096);
  options.Add("OverwriteDef", 0);
  options.Add("CopyStreamsDef", 0);
  options.Add("CopyRightsDef", 0);
  options.Add("AllowParallel", 1);
  options.Add("DefParallel", 0);
  options.Add("CopyDescs", 1);
  options.Add("DescsInSubdirs", 0);
  options.Add("ConnectLikeBars", 0);
  options.Add("ConfirmBreak", 1);
  options.Add("Sound", 0);
  options.Add("PreallocMin", 4096);
  options.Add("UnbuffMin", 64);
  options.Add("ReadFilesOpenedForWriting", 0);
  options.Add("CheckFreeDiskSpace", 1);
}

FarPlugin* InitInstance()
{
  return new FileCopyExPlugin;
}