#if !defined(_MSC_VER)
#include <sec_api/wchar_s.h>
#endif
#include "FarMacro.h"

#include "Framework/StringVector.h"
#include "Framework/FileUtils.h"
#include "Common.h"
#include "guid.hpp"

String base;

String getEnv(const String & name)
{
  wchar_t * buf;
  size_t len;
  if (_wdupenv_s(&buf, &len, name.c_str()) == 0)
  {
    String v(buf);
    free(buf);
    return v;
  }
  return L"";
}

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

  if (base.empty())
  {
    base = getEnv(L"FARPROFILE");
  }
  if (base.empty())
  {
    base = getEnv(L"APPDATA");
    if (!base.empty())
    {
      base += L"\\Far Manager\\Profile";
    }
  }
  if (base.empty())
  {
    return; // Cannot find correct path
  }
  String fname = String(L"Shell_") + key + L".lua";
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
  String DirName = base + L"\\Macros\\scripts\\";
  if (!FileExists(DirName))
  {
    ForceDirectories(DirName);
  }
  v.saveToFile(DirName + fname);

  Info.MacroControl(&MainGuid, MCTL_LOADALL, 0, nullptr);
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
