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

#include "Framework/stdhdr.h"
//#include "Framework/lowlevelstr.h"
#include "Framework/properties.h"
#include "Framework/FileUtils.h"
#include "Engine.h"
#include "FarMenu.h"
#include "FarPlugin.h"
#include "version.hpp"
#include "common.h"
#include "guid.hpp"

#include "ui.h"

//const String menu_plug = L'code="Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", )';

void Bind(const String& key, const String& seq, const String& desc, void *id)
{
	/* XXX
	String src = regkey + "\\" + key;
	registry.SetString(src, "Sequence", seq);
	+registry.SetInt(src, "DisableOutput", 1);
	registry.SetInt(src, "NoPluginPanels", 0);
	registry.SetInt(src, "PluginPanels", 1); 
	*/
	
	MacroAddMacro macro = { sizeof(MacroAddMacro) };
	macro.Id = id;
	macro.SequenceText = seq.c_str();
	macro.Description = desc.c_str(); // !!! put something
	macro.Flags = KMFLAGS_DISABLEOUTPUT;
	INPUT_RECORD ir;
	int resKey = FSF.FarNameToInputRecord(key.c_str(), &ir);
	macro.AKey = ir;
	//ShowMessage("Bind - key convert", String(resKey), FMSG_MB_OK);
	macro.Area = MACROAREA_SHELL;
	macro.Callback = NULL;
	int res = Info.MacroControl(&MainGuid, MCTL_ADDMACRO, 0, &macro);
	ShowMessage("Bind", key + L": " + String(res), FMSG_MB_OK);
}

void BindAll()
{
	Bind("F5", "Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 0)", "", (void*)0);
	Bind("F6", "Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 1)", "", (void*)1);
	Bind("ShiftF5", "Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 2)", "", (void*)2);
	Bind("ShiftF6", "Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 3)", "", (void*)3);
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
		LoadLocale(GetDLLPath() + "\\resource\\" + fn, locale);
	}
}

void FarPlugin::Create()
{
	// bug #15 fixed by Ivanych
	InitLang();

	if(!dialogs.Load(GetDLLPath() + "\\resource\\dialogs.objd"))
	{
		FWError("Could not load dialogs.objd");
		exit(0);
	}
	InitOptions();
	settings.create();
	LoadOptions();

	if (options["BindF5F6"]) {
		BindAll();
	};

	// XXX descs.LoadFromString(registry.GetString("\\Software\\Far2\\Descriptions", "ListNames", "Descript.ion,Files.bbs"), ',');
	// XXX Workaround
	descs.AddString("Descript.ion");
	descs.AddString("Files.bbs");
}

void FarPlugin::LoadOptions()
{
	loadOptions(options, settings);
}

void FarPlugin::SaveOptions()
{
	saveOptions(options, settings);
}

int FarPlugin::Configure(const struct ConfigureInfo *Info)
{
	Config();
	return TRUE;
}

void CallCopy(int move, int curOnly)
{
	Engine engine;
	Engine::MResult res=engine.Main(move, curOnly);
	if(res == Engine::MRES_STDCOPY || res == Engine::MRES_STDCOPY_RET)
	{
		/* svs 09.02.2011 18:51:58 +0300 - build 1844                                                                                                                                                     ░
                                                                                                                                                                                               ░
2. Удален ACTL_POSTKEYSEQUENCE (есть аналог в лице ACTL_KEYMACRO).                                                                                                                             ░
   FARKEYSEQUENCEFLAGS переименован в FARKEYMACROFLAGS (и KSFLAGS_* -> KMFLAGS_* ).                                                                                                            ░
   Удалена структура KeySequence.                                                                                                                                                              ░
   */
		/* XXX KeySequence seq;
		DWORD keys[8];
		seq.Flags=KSFLAGS_DISABLEOUTPUT;
		seq.Sequence=keys;
		seq.Count=1;
		keys[0]=move ? KEY_F6 : KEY_F5;
		if(res == Engine::MRES_STDCOPY_RET)
		{
			seq.Count=2;
			keys[1]=KEY_ENTER;
		}
		Info.AdvControl(Info.ModuleNumber, ACTL_POSTKEYSEQUENCE, (void*)&seq);
		*/
	}
}

void FarPlugin::OpenPlugin(const struct OpenInfo *OInfo)
{
	int command = -1;

	if (OInfo->OpenFrom == OPEN_FROMMACRO) {
		OpenMacroInfo *macroInfo = (OpenMacroInfo*)OInfo->Data;
		if (macroInfo->Count >= 1) {
			FarMacroValue &v = macroInfo->Values[0];
			if (v.Type == FMVT_DOUBLE) {
				command = v.Double;
			};
		};
	} 
	if (command == -1) {
		FarMenu menu;
		menu.SetFlags(FMENU_WRAPMODE);
		menu.SetTitle(LOC("PluginName"));
		menu.SetHelpTopic("Menu");
		menu.AddLine(LOC("Menu.CopyFiles"));
		menu.AddLine(LOC("Menu.MoveFiles"));
		menu.AddLine(LOC("Menu.CopyFilesUnderCursor"));
		menu.AddLine(LOC("Menu.MoveFilesUnderCursor"));
		menu.AddSep();
		menu.AddLine(LOC("Menu.Config"));
		command = menu.Execute();
	}

	int move=0, curOnly=0;
	switch (command)
	{
		case 0: move = 0; curOnly = 0; break;
		case 1: move = 1; curOnly = 0; break;
		case 2: move = 0; curOnly = 1; break;
		case 3: move = 1; curOnly = 1; break;
		case 5: Config(); return; 
		default: return;
	}
	CallCopy(move, curOnly);
}

String FarPlugin::GetDLLPath()
{
	wchar_t buf[MAX_FILENAME];
	GetModuleFileName((HMODULE)hInstance, buf, MAX_FILENAME);
	String dlln = buf;
	return dlln.substr(0, dlln.rfind('\\'));
}


void FarPlugin::InitOptions()
{
	options["BufPercent"] = 1;
	options["BufSize"] = 0;
	options["BufPercentVal"] = 15;
	options["BufSizeVal"] = 4096;
	options["OverwriteDef"] = 0;
	options["CopyStreamsDef"] = 0;
	options["CopyRightsDef"] = 0;
	options["AllowParallel"] = 1;
	options["DefParallel"] = 1;
	options["CopyDescs"] = 1;
	options["DescsInSubdirs"] = 0;
	options["ConnectLikeBars"] = 0;
	options["ConfirmBreak"] = 1;
	options["Sound"] = 1;
	options["PreallocMin"] = 64;
	options["UnbuffMin"] = 64;
	options["ReadFilesOpenedForWriting"] = 1;
	options["CheckFreeDiskSpace"] = 1;
	options["BindF5F6"] = 1;
}


int FarPlugin::Binded(const String& key)
{
	//XXX String seq = registry.GetString(regkey + "\\" + key, "Sequence", "");
	//XXX return (!seq.nicmp(menu_plug, menu_plug.len()) || !seq.icmp("F5") || !seq.icmp("F6"));
	return true;
}

void FarPlugin::Bind(const String& key, const String& seq, const String& desc, void *id)
{
	/* XXX
	String src = regkey + "\\" + key;
	registry.SetString(src, "Sequence", seq);
	+registry.SetInt(src, "DisableOutput", 1);
	registry.SetInt(src, "NoPluginPanels", 0);
	registry.SetInt(src, "PluginPanels", 1); 
	*/
	
	MacroAddMacro macro = { sizeof(MacroAddMacro) };
	macro.Id = id;
	macro.SequenceText = seq.c_str();
	macro.Description = desc.c_str(); // !!! put something
	macro.Flags = KMFLAGS_DISABLEOUTPUT;
	INPUT_RECORD ir;
	int resKey = FSF.FarNameToInputRecord(key.c_str(), &ir);
	macro.AKey = ir;
	//ShowMessage("Bind - key convert", String(resKey), FMSG_MB_OK);
	macro.Area = MACROAREA_SHELL;
	macro.Callback = NULL;
	int res = Info.MacroControl(&MainGuid, MCTL_ADDMACRO, 0, &macro);
	ShowMessage("Bind", key + L": " + String(res), FMSG_MB_OK);
}

void FarPlugin::Unbind(void *id)
{
	Info.MacroControl(&MainGuid, MCTL_DELMACRO, 0, id);
}

// XXX I don't know how to replace FARMACROCOMMAND
/*
void FarPlugin::MacroCommand(const FARMACROCOMMAND& cmd)
{
	ActlKeyMacro prm;
	memset(&prm, 0, sizeof(prm));
	prm.Command = cmd;
	Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &prm);
}
*/

void FarPlugin::KeyConfig()
{
	int res = Info.MacroControl(&MainGuid, MCTL_SAVEALL, 0, NULL);
	ShowMessage("Bind - SaveAll", String(res), FMSG_MB_OK);
	return;

	FarDialog& dlg = Dialogs()["KeysDialog"];
	dlg.ResetControls();

	bool bind = Binded("F5") && Binded("F6") && Binded("ShiftF5") && Binded("ShiftF6");
	bool altShift = bind && Binded("AltShiftF5") && Binded("AltShiftF6");
	bool ctrlShift = bind && Binded("CtrlShiftF5") && Binded("CtrlShiftF6");
	bool ctrlAlt = bind && Binded("CtrlAltF5") && Binded("CtrlAltF6");
	if(!altShift && !ctrlShift && !ctrlAlt)
		altShift = true;

	dlg["BindToF5"]("Selected") = bind;
	dlg["AltShiftF5"]("Selected") = altShift;
	dlg["CtrlShiftF5"]("Selected") = ctrlShift;
	dlg["CtrlAltF5"]("Selected") = ctrlAlt;

	if(dlg.Execute() == -1)
		return;

	if(dlg["BindToF5"]("Selected") == bind
		&& dlg["AltShiftF5"]("Selected") == altShift
		&& dlg["CtrlShiftF5"]("Selected") == ctrlShift
		&& dlg["CtrlAltF5"]("Selected") == ctrlAlt)
		return;

	// MacroCommand(MCMD_SAVEALL); // XXX

	Unbind("KEY_F5");	Unbind("ShiftF5");
	Unbind("KEY_F5");			Unbind("ShiftF6");
	Unbind("AltShiftF5");	Unbind("AltShiftF6");
	Unbind("CtrlShiftF5");	Unbind("CtrlShiftF6");
	Unbind("CtrlAltF5");	Unbind("CtrlAltF6");

	if(dlg["BindToF5"]("Selected"))
	{
		Bind("F5", "Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 0)", "", 0);
		Bind("F6", "Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 1)", "", 0);
		Bind("ShiftF5", "Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 0)", "", 0);
		Bind("ShiftF6", "Plugin.Call(\"16990c75-cb7a-43df-8d7e-d6bf3683c3f1\", 1)", "", 0);

		/*
		String key;
		if(dlg["AltShiftF5"]("Selected")) key = "AltShift";
		else if(dlg["CtrlShiftF5"]("Selected")) key = "CtrlShift";
		else if(dlg["CtrlAltF5"]("Selected")) key = "CtrlAlt";
		Bind(key + "F5", "F5");
		Bind(key + "F6", "F6");
		*/
	}
	// MacroCommand(MCMD_LOADALL); // XXX
}

#define VersionStr0(a,b,c,d, bit) "version " #a "." #b "." #c "." #d " beta (" #bit " Unicode), " __DATE__
#ifdef _WIN64
#define VersionStr(a,b,c,d) VersionStr0(a,b,c,d, x64) 
#else
#define VersionStr(a,b,c,d) VersionStr0(a,b,c,d, x32) 
#endif

void FarPlugin::About()
{
	FarDialog& dlg = Dialogs()["AboutDialog"];
	dlg.ResetControls();
	dlg["Label2"]("Text") = String(VersionStr(FARMANAGERVERSION_MAJOR, FARMANAGERVERSION_MINOR, PLUGIN_MAJOR, PLUGIN_BUILD));
	dlg.Execute();
}

void beep(int b)
{
	switch(b)
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

void FarPlugin::Config()
{
	FarDialog& dlg = Dialogs()["SetupDialog"];
	dlg.ResetControls();
	dlg.LoadState(options);

rep:
	int res = dlg.Execute();
	switch(res)
	{
		case 0:
			dlg.SaveState(options);
			SaveOptions();
			break;
		case 1:
			KeyConfig();
			goto rep;
		case 2:
			About();
			goto rep;
		case 3:
			static int bn = 0;
			beep(bn);
			if (++bn > 2) {
				bn = 0;
			}
			goto rep;
	}
}
