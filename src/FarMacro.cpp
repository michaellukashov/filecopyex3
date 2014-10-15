#if !defined(_MSC_VER)
#include <sec_api/wchar_s.h>
#endif
#include "FarMacro.h"

#include "Framework/StringVector.h"
#include "Framework/FileUtils.h"
#include "Common.h"
#include "version.hpp"
#include "guid.hpp"

void Bind(const String & key, const String & code, const String & desc, int id)
{
  String DirName = GetMacrosPath(PLUGIN_BUILD);
  if (DirName.empty())
  {
    return; // Cannot find correct path
  }
  if (!FileExists(DirName))
  {
    ForceDirectories(DirName);
  }
  String fname = GetMacroFileName(key);
  StringVector v;
  v.AddString(String(L"Macro {"));
  v.AddString(String(L"  area=\"Shell\";"));
  v.AddString(String(L"  key=\"") + key + L"\";");
  v.AddString(String(L"  flags=\"NoPluginPanels|NoPluginPPanels|NoSendKeysToPlugins\";"));
  v.AddString(String(L"  priority=0;"));
  v.AddString(String(L"  description=\"") + desc + L"\";");
  v.AddString(String(L"  action = function()"));
  v.AddString(String(L"    ") + code);
  v.AddString(String(L"  end;"));
  v.AddString(String(L"}"));

  v.saveToFile(DirName + fname);

  Info.MacroControl(&MainGuid, MCTL_LOADALL, 0, nullptr);

  FarSettings & settings = plugin->Settings();
  settings.set(key, L"true");
  PropertyMap & options = plugin->Options();
  options[key] = Property(L"true");
}

int Bound(const String & key)
{
  FarSettings & settings = plugin->Settings();
  String value;
  return settings.get(key, value) && !value.IsEmpty();
}

void Unbind(const String & key)
{
  String MacrosPath = GetMacrosPath(PLUGIN_BUILD);
  if (FileExists(MacrosPath))
  {
    RemoveMacroFile(MacrosPath, key);
  }

  FarSettings & settings = plugin->Settings();
  settings.set(key, L"");
  PropertyMap & options = plugin->Options();
  options[key] = Property(L"");
  //Info.MacroControl(&MainGuid, MCTL_DELMACRO, 0, id);
}

String GetMacrosPath(uintptr_t PluginBuildNumber)
{
  String ProfilePath = GetFarProfilePath();
  if (ProfilePath.empty())
  {
    return L""; // Cannot find correct path
  }
  String DirName;
  if (PluginBuildNumber <= 20)
  {
    DirName = AddEndSlash(AddEndSlash(ProfilePath) + L"Macros\\internal");
  }
  else
  {
    DirName = AddEndSlash(AddEndSlash(ProfilePath) + L"Macros\\scripts");
  }
  return DirName;
}

String GetMacroFileName(const String & Key)
{
  return String(L"Shell_") + Key + L".lua";
}

void RemoveMacroFile(const String & MacrosPath, const String & Key)
{
  String MacroFileName = GetMacroFileName(Key);
  String FullMacroFileName = AddEndSlash(MacrosPath) + MacroFileName;
  if (FileExists(FullMacroFileName))
  {
    // check if macros are created by plugin
    StringVector v;
    v.loadFromFile(FullMacroFileName);
    if (v.FindAny(L"FileCopyEx3") != -1)
    {
      FDelete(FullMacroFileName);
    }
  }
}
