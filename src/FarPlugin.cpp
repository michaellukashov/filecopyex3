﻿/*
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

#include "Framework/StdHdr.h"
#include "Framework/Properties.h"
#include "Framework/FileUtils.h"
#include "Engine.h"
#include "FarMacro.h"
#include "FarMenu.h"
#include "FarPlugin.h"
#include "Common.h"
#include "version.hpp"
#include "guid.hpp"

#include "ui.h"

static void BindAll()
{
  Bind(L"F5", L"if not Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 0) then Keys(\"F5\") end", L"FileCopyEx3 - Copy file(s)", 0);
  Bind(L"F6", L"if not Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 1) then Keys(\"F6\") end", L"FileCopyEx3 - Move file(s)", 0);
  Bind(L"ShiftF5", L"if not Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 2) then Keys(\"ShiftF5\") end", L"FileCopyEx3 - Copy current file", 0);
  Bind(L"ShiftF6", L"if not Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 3) then Keys(\"ShiftF6\") end", L"FileCopyEx3 - Move current file", 0);
}

FarPlugin::~FarPlugin()
{
}

void FarPlugin::InitLang()
{
  String fn = GetMsg(0);
  if (fn != CurLocaleFile)
  {
    CurLocaleFile = fn;
    LoadLocale(GetDLLPath() + L"\\resource\\" + fn, locale);
  }
}

void FarPlugin::Create()
{
  InitLang();

  if (!dialogs.Load(GetDLLPath() + L"\\resource\\dialogs.objd"))
  {
    FWError(L"Could not load dialogs.objd");
    exit(0);
  }
  InitOptions();
  LoadOptions();

  descs.AddString(L"Descript.ion");
  descs.AddString(L"Files.bbs");
  UpdateConfiguration();
}

void FarPlugin::LoadOptions()
{
  loadOptions(options, settings);
}

void FarPlugin::SaveOptions()
{
  saveOptions(options, settings);
}

intptr_t FarPlugin::Configure(const struct ConfigureInfo * Info)
{
  Config();
  return TRUE;
}

static bool CallCopy(bool move, bool curOnly)
{
  Engine engine;
  Engine::MResult res = engine.Main(move, curOnly);
  bool Result = res == Engine::MRES_OK;
  if (res == Engine::MRES_STDCOPY || res == Engine::MRES_STDCOPY_RET)
  {
  }
  return Result;
}

void Beep(int b, bool useBASS, const String &file)
{
  if (useBASS && !file.IsEmpty())
  {
    String start = L"local function LoadBASS() "
      L"require('LuaBASS')"
      L"end;"
      L"if pcall(LoadBASS) then "
      L"LuaBASS.Init(1,44100,0);"
      L"local Handle = LuaBASS.StreamCreateFile('";
    start += file.replace(L"\\", L"\\\\");
    start += L"',0,0,LuaBASS.Flags(LuaBASS.STREAM.BASS_STREAM_PRESCAN));"
      L"LuaBASS.ChannelPlay(Handle,false) "
      L"end";
    MacroExecuteString mes = { sizeof(mes) };
    mes.Flags = KMFLAGS_LUA;
    mes.SequenceText = start.c_str();
    Info.MacroControl(&MainGuid, MCTL_EXECSTRING, 0, &mes);
  }
  else
  {
    switch (b)
    {
    case 0:
      MessageBeep(MB_ICONWARNING);
      break;

    case 1:
      MessageBeep(MB_ICONASTERISK);
      break;

    case 2:
      MessageBeep(MB_OK);
      break;
    }
  }
}

void Beep(int b)
{
  PropertyMap & Options = plugin->Options();
  Beep(b, Options[L"UseBASS"], Options[L"AudioFile"]);
}

void FarPlugin::OpenPlugin(const struct OpenInfo * OInfo)
{
  intptr_t command = -1;

  if (OInfo->OpenFrom == OPEN_FROMMACRO)
  {
    OpenMacroInfo * macroInfo = reinterpret_cast<OpenMacroInfo *>(OInfo->Data);
    if (macroInfo->Count >= 1)
    {
      FarMacroValue & v = macroInfo->Values[0];
      if (v.Type == FMVT_DOUBLE)
      {
        command = static_cast<intptr_t>(v.Double);
      }
    }
  }
  if (command == -1)
  {
    bool repeat = true;
    size_t selectedMenuItem = 0;
    while (repeat)
    {
      FarMenu menu;
      menu.SetFlags(FMENU_WRAPMODE);
      menu.SetTitle(LOC(L"PluginName"));
      menu.SetHelpTopic(L"Menu");
      menu.AddLine(LOC(L"Menu.CopyFiles"));
      menu.AddLine(LOC(L"Menu.MoveFiles"));
      menu.AddLine(LOC(L"Menu.CopyFilesUnderCursor"));
      menu.AddLine(LOC(L"Menu.MoveFilesUnderCursor"));
      menu.AddSep();
      menu.AddLine(LOC(L"Menu.Config"));
      menu.SetSelection(selectedMenuItem);
      command = menu.Execute();

      bool move, curOnly;
      bool copy = true;
      switch (command)
      {
      case 0:
        move = false;
        curOnly = false;
        selectedMenuItem = 0;
        repeat = false;
        break;
      case 1:
        move = true;
        curOnly = false;
        selectedMenuItem = 1;
        repeat = false;
        break;
      case 2:
        move = false;
        curOnly = true;
        selectedMenuItem = 2;
        repeat = false;
        break;
      case 3:
        move = true;
        curOnly = true;
        selectedMenuItem = 3;
        repeat = false;
        break;
      case 5:
        Config();
        copy = false;
        selectedMenuItem = 5;
        repeat = true;
        break;
      default:
        return;
      }
      if (copy)
      {
        if (!CallCopy(move, curOnly))
          repeat = true;
      }
    }
  }
  else
  {
    switch (command)
    {
    case 0:
      CallCopy(false, false);
      break;
    case 1:
      CallCopy(true, false);
      break;
    case 2:
      CallCopy(false, true);
      break;
    case 3:
      CallCopy(true, true);
      break;
    default:
      break;
    }
  }
}

String FarPlugin::GetDLLPath()
{
  wchar_t buf[MAX_FILENAME];
  ::GetModuleFileName((HMODULE)hInstance, buf, _countof(buf));
  String dlln = buf;
  return dlln.substr(0, dlln.rfind('\\'));
}

void FarPlugin::UpdateConfiguration()
{
  // check if Macros\\internal macros exist
  {
    String MacrosPath = GetMacrosPath(20);
    if (FileExists(MacrosPath))
    {
      RemoveMacroFile(MacrosPath, L"F5");
      RemoveMacroFile(MacrosPath, L"F6");
      RemoveMacroFile(MacrosPath, L"ShiftF5");
      RemoveMacroFile(MacrosPath, L"ShiftF6");
    }
  }
}

void FarPlugin::InitOptions()
{
  PropertyMap & Options = this->Options();
  Options[L"BufPercent"] = 0;
  Options[L"BufSize"] = 1;
  Options[L"BufPercentVal"] = 15;
  Options[L"BufSizeVal"] = DEFAULT_SECTOR_SIZE;
  Options[L"OverwriteDef"] = 0;
  Options[L"CopyStreamsDef"] = 0;
  Options[L"CopyRightsDef"] = 0;
  Options[L"AllowParallel"] = 1;
  Options[L"DefParallel"] = 1;
  Options[L"CopyDescs"] = 1;
  Options[L"DescsInSubdirs"] = 0;
  Options[L"ConnectLikeBars"] = 0;
  Options[L"ConfirmBreak"] = 1;
  Options[L"Sound"] = 1;
  Options[L"PreallocMin"] = 64;
  Options[L"UnbuffMin"] = 64;
  Options[L"ReadFilesOpenedForWriting"] = 1;
  Options[L"CheckFreeDiskSpace"] = 1;
  Options[L"copyCreationTime"] = 1;
  Options[L"copyLastAccessTime"] = 1;
  Options[L"copyLastWriteTime"] = 1;
  Options[L"compressDef"] = 2;
  Options[L"encryptDef"] = 2;
  Options[L"readSpeedLimitDef"] = 0;
  Options[L"readSpeedLimitValueDef"] = String(L"");
  Options[L"writeSpeedLimitDef"] = 0;
  Options[L"writeSpeedLimitLimitDef"] = String(L"");
  Options[L"UseBASS"] = 0;
  Options[L"AudioFile"] = String(L"");
};

void FarPlugin::KeyConfig()
{
  //int res = Info.MacroControl(&MainGuid, MCTL_SAVEALL, 0, nullptr);
  //
  //BindAll();
  //ShowMessage(L"", L"Hotkeys binded", FMSG_MB_OK);
  //return;
  FarDialog & dlg = plugin->Dialogs()[L"KeysDialog"];
  dlg.ResetControls();

  bool bind = Bound(L"F5") && Bound(L"F6") && Bound(L"ShiftF5") && Bound(L"ShiftF6");
  bool altShift = bind && Bound(L"AltShiftF5") && Bound(L"AltShiftF6");
  bool ctrlShift = bind && Bound(L"CtrlShiftF5") && Bound(L"CtrlShiftF6");
  bool ctrlAlt = bind && Bound(L"CtrlAltF5") && Bound(L"CtrlAltF6");
  if (!altShift && !ctrlShift && !ctrlAlt)
    altShift = true;

  dlg[L"BindToF5"](L"Selected") = bind;
  dlg[L"AltShiftF5"](L"Selected") = altShift;
  dlg[L"CtrlShiftF5"](L"Selected") = ctrlShift;
  dlg[L"CtrlAltF5"](L"Selected") = ctrlAlt;

  if (dlg.Execute() == -1)
    return;

  if (dlg[L"BindToF5"](L"Selected") == bind &&
      dlg[L"AltShiftF5"](L"Selected") == altShift &&
      dlg[L"CtrlShiftF5"](L"Selected") == ctrlShift &&
      dlg[L"CtrlAltF5"](L"Selected") == ctrlAlt)
    return;

  Unbind(L"F5"); Unbind(L"ShiftF5");
  Unbind(L"F6");     Unbind(L"ShiftF6");
  Unbind(L"AltShiftF5"); Unbind(L"AltShiftF6");
  Unbind(L"CtrlShiftF5");  Unbind(L"CtrlShiftF6");
  Unbind(L"CtrlAltF5");  Unbind(L"CtrlAltF6");

  if (dlg[L"BindToF5"](L"Selected"))
  {
    BindAll();

    String key;
    if (dlg[L"AltShiftF5"](L"Selected"))
      key = L"AltShift";
    else if (dlg[L"CtrlShiftF5"](L"Selected"))
      key = L"CtrlShift";
    else if (dlg[L"CtrlAltF5"](L"Selected"))
      key = L"CtrlAlt";
    Bind(key + L"F5", L"Keys(\"F5\")", L"FileCopyEx3 - Standard copy dialog", 0);
    Bind(key + L"F6", L"Keys(\"F6\")", L"FileCopyEx3 - Standard move dialog", 0);
  }
}

void FarPlugin::SoundConfig()
{
  static int bn = 0;
  FarDialog & dlg = plugin->Dialogs()[L"SoundDialog"];
  dlg.ResetControls();
  dlg.LoadState(options);

  bool useBASS = dlg[L"UseBASS"](L"Selected");
  dlg[L"UseSystem"](L"Selected") = !useBASS;

  while (true)
  {
    intptr_t res = dlg.Execute();
    switch (res)
    {
    case 0:
      dlg.SaveState(options);
      return;
    case 1:
      Beep(bn, dlg[L"UseBASS"](L"Selected"), dlg[L"AudioFile"](L"Text"));
      if (++bn > 2)
      {
        bn = 0;
      }
      break;
    default:
      return;
    }
  }
}

#define VersionStr0(a,b,c,d, bit) "version " #a "." #b "." #c "." #d " beta (" #bit " Unicode), " __DATE__
#ifdef _WIN64
#define VersionStr(a,b,c,d) VersionStr0(a,b,c,d, x64)
#else
#define VersionStr(a,b,c,d) VersionStr0(a,b,c,d, x32)
#endif

void FarPlugin::About()
{
  FarDialog & dlg = Dialogs()[L"AboutDialog"];
  dlg.ResetControls();
  dlg[L"Label2"](L"Text") = String(VersionStr(PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_SUBMINOR, PLUGIN_BUILD));
  dlg.Execute();
}

void FarPlugin::Config()
{
  FarDialog & dlg = Dialogs()[L"SetupDialog"];
  dlg.ResetControls();
  dlg.LoadState(options);

  bool Repeat = true;
  while (Repeat)
  {
    Repeat = false;
    intptr_t res = dlg.Execute();
    switch (res)
    {
      case 0:
        dlg.SaveState(options);
        SaveOptions();
        Repeat = false;
        break;
      case 1:
        KeyConfig();
        break;
      case 2:
        About();
        break;
      case 3:
        SoundConfig();
        break;
    }
  }
}
