#pragma once
#include "../framework/properties.h"
#include "../framework/far/farplugin.h"

#define MRES_NONE         0
#define MRES_STDCOPY      1
#define MRES_STDCOPY_RET  2

extern int Win2K, WinNT;
extern StringList DescFiles;

class FileCopyExPlugin :
  public FarPlugin
{
public:
  FileCopyExPlugin(void);
  virtual ~FileCopyExPlugin(void);
  int Configure(int);
  FarPanel* OpenPlugin(int, int);
  void InitOptions(PropertyList&);
  void Config();

private:
  void About();
  void KeyConfig();
  void MiscInit();
};

void SaveTemp();
extern StringList TempFiles;
