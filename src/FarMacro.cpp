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
  /*
  MacroAddMacro macro = { sizeof(MacroAddMacro) };
  macro.Id = id;
  macro.SequenceText = seq.c_str();
  macro.Description = desc.c_str(); // !!! put something
  macro.Flags = KMFLAGS_DISABLEOUTPUT;
  INPUT_RECORD ir;
  int resKey = FSF.FarNameToInputRecord(key.c_str(), &ir);
  macro.AKey = ir;
  //ShowMessage(L"Bind - key convert", String(resKey), FMSG_MB_OK);
  macro.Area = MACROAREA_SHELL;
  macro.Callback = nullptr;
  int res = Info.MacroControl(&MainGuid, MCTL_ADDMACRO, 0, &macro);
  ShowMessage(L"Bind", key + L": " + String(res), FMSG_MB_OK);
  */
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
}

int Binded(const String & key)
{
  FarSettings & settings = plugin->Settings();
  //XXX String seq = registry.GetString(regkey + L"\\" + key, "Sequence", "");
  //XXX return (!seq.nicmp(menu_plug, menu_plug.len()) || !seq.icmp(L"F5") || !seq.icmp(L"F6"));
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
  //Info.MacroControl(&MainGuid, MCTL_DELMACRO, 0, id);
}

/*
void FarPlugin::MacroCommand(const FARMACROCOMMAND& cmd)
{
  ActlKeyMacro prm;
  memset(&prm, 0, sizeof(prm));
  prm.Command = cmd;
  Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &prm);
}
*/

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
      Delete(FullMacroFileName);
    }
  }
}
