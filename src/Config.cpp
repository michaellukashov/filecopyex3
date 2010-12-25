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

#include "../framework/stdhdr.h"
#include "../framework/lowlevelstr.h"
#include "filecopyex.h"
#include "../framework/far/interface/farkeys.hpp"

const String regkey = "\\Software\\Far2\\KeyMacros\\Shell";
const String menu_plug = "F11 $if(menu.Select(\"Extended copy\",2)<=0) MsgBox(\"Extended copy\",\"Plugin was not found in list!\",1) $Exit $end Enter ";

int FileCopyExPlugin::Binded(const String& key)
{
	String seq = registry.GetString(regkey + "\\" + key, "Sequence", "");
	return (!seq.nicmp(menu_plug, menu_plug.len()) || !seq.icmp("F5") || !seq.icmp("F6"));
}

void FileCopyExPlugin::Bind(const String& key, const String& seq)
{
	String src = regkey + "\\" + key;
	registry.SetString(src, "Sequence", seq);
	registry.SetInt(src, "DisableOutput", 1);
	registry.SetInt(src, "NoPluginPanels", 0);
	registry.SetInt(src, "PluginPanels", 1); 
}

void FileCopyExPlugin::Unbind(const String& key)
{
	registry.DeleteKey(regkey + "\\" + key);
}

void FileCopyExPlugin::MacroCommand(const FARMACROCOMMAND& cmd)
{
	ActlKeyMacro prm;
	memset(&prm, 0, sizeof(prm));
	prm.Command = cmd;
	Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &prm);
}

void FileCopyExPlugin::KeyConfig()
{
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

	MacroCommand(MCMD_SAVEALL);

	Unbind("F5");			Unbind("ShiftF5");
	Unbind("F6");			Unbind("ShiftF6");
	Unbind("AltShiftF5");	Unbind("AltShiftF6");
	Unbind("CtrlShiftF5");	Unbind("CtrlShiftF6");
	Unbind("CtrlAltF5");	Unbind("CtrlAltF6");

	if(dlg["BindToF5"]("Selected"))
	{
		Bind("F5", menu_plug + "1");
		Bind("F6", menu_plug + "2");
		Bind("ShiftF5", menu_plug + "3"); 
		Bind("ShiftF6", menu_plug + "4");
		String key;
		if(dlg["AltShiftF5"]("Selected")) key = "AltShift";
		else if(dlg["CtrlShiftF5"]("Selected")) key = "CtrlShift";
		else if(dlg["CtrlAltF5"]("Selected")) key = "CtrlAlt";
		Bind(key + "F5", "F5");
		Bind(key + "F6", "F6");
	}
	MacroCommand(MCMD_LOADALL);
}

#ifdef _WIN64
#define VersionStr "version 2.0.0 beta (x64 Unicode), " __DATE__
#else
#define VersionStr "version 2.0.0 beta (x86 Unicode), " __DATE__
#endif

void FileCopyExPlugin::About()
{
	FarDialog& dlg = Dialogs()["AboutDialog"];
	dlg.ResetControls();
	dlg["Label2"]("Text") = String(VersionStr);
	dlg.Execute();
}

void beep(int b)
{
	switch(b)
	{
	case 0:		MessageBeep(MB_ICONWARNING);	break;
	case 1:		MessageBeep(MB_ICONASTERISK);	break;
	case 2:		MessageBeep(MB_OK);				break;
	}
}

void FileCopyExPlugin::Config()
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
		if(++bn > 2)
			bn = 0;
		goto rep;
	}
}
