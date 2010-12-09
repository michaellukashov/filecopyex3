#pragma once
#include "far/interface/plugin.hpp"
#include "array.h"
#include "objstring.h"
#include "fileutils.h"

class FarPanel
{
public:
  FarPanel();
  virtual ~FarPanel();

  void CallGetOpenPluginInfo(OpenPluginInfo *info);
  int CallSetDirectory(const String& dir, int opmode);
  int CallGetFindData(PluginPanelItem* &items, int &nitems, int opmode);
  void CallFreeFindData(PluginPanelItem *items, int nitems);
  int CallDeleteFiles(PluginPanelItem *items, int nitems, int opmode);
  int CallPutFiles(PluginPanelItem *items, int nitems, int move, int opmode);
  int CallGetFiles(PluginPanelItem *items, int nitems, int move, 
    const String& dest, int opmode);
  int CallMakeDirectory(String& name, int opmode);

  int Flags;
  String FormatName, PanelTitle, CurDir;
  Array<PluginPanelItem> Files;

  virtual int ChangeDir(String& dir, int silent, const String& suggest);
  virtual int ReadFileList(int silent);
  virtual int DelFiles(PluginPanelItem* files, int count, int silent);
  virtual int PutFiles(PluginPanelItem* files, int count, int move, int silent);
  virtual int GetFiles(PluginPanelItem* files, int count, int move, 
    const String& dest, int silent);
  virtual int MkDir(String& name, int silent);

private:
  char _TitleBuf[128], _FormatBuf[128], _CurDirBuf[MAX_FILENAME];
  int Rescan;
};
