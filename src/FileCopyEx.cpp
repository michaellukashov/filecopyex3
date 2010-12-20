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
#include "common.h"
#include "engine.h"
#include "filecopyex.h"
#include "../framework/far/interface/farkeys.hpp"

void FileCopyExPlugin::Create()
{
	MenuItems.Add("E&xtended copy");
	ConfigItems.Add("Extended copy");
	FarPlugin::Create();
	descs.LoadFromString(registry.GetString("\\Software\\Far2\\Descriptions", "ListNames", "Descript.ion,Files.bbs"), ',');
}

FileCopyExPlugin::~FileCopyExPlugin(void)
{
}

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

void FileCopyExPlugin::OpenPlugin(int from, int item)
{
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
	menu.AddLine(LOC("Menu.Config"));

	int move, curonly;
	switch (menu.Execute())
	{
	case 0: move = 0; curonly = 0; break;
	case 1: move = 1; curonly = 0; break;
	case 2: move = 0; curonly = 1; break;
	case 3: move = 1; curonly = 1; break;
	case 5: Config(); return; 
	default: return;
	}
	CallCopy(move, curonly);
}

void FileCopyExPlugin::InitOptions()
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

FarPlugin* CreatePlugin()
{
	FarPlugin* p = new FileCopyExPlugin;
	p->Create();
	return p;
}

FileCopyExPlugin* Plugin()
{
	return (FileCopyExPlugin*)plugin;
}
