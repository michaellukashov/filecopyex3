#pragma once
#include "far/farplugin.h"

class TempPanel : public FarPanel
{
public:
  TempPanel();
  ~TempPanel();

  int ReadFileList(int silent);
  int ChangeDir(String& dir, int silent, const String& suggest);
  int DelFiles(PluginPanelItem* files, int count, int silent);
  int PutFiles(PluginPanelItem* files, int count, int move, int silent);
  int GetFiles(PluginPanelItem* files, int count, int move, 
    const String& dest, int silent);
  int MkDir(String& name, int silent);

};
