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

#include <Shobjidl.h>
#include "taskbar_icon.h"

TaskBarIcon::TaskBarIcon() : tbl(NULL), last_state(S_NO_PROGRESS)
{
	CoInitialize(NULL);
	CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&tbl));
}
TaskBarIcon::~TaskBarIcon()
{
	if(last_state != S_NO_PROGRESS)
		SetState(S_NO_PROGRESS);
	if(tbl)
		tbl->Release();
	CoUninitialize();
}
void TaskBarIcon::SetState(State state, float param)
{
	if(!tbl)
		return;
	HWND hwnd = GetConsoleWindow();
	if(!hwnd)
		return;
	switch(state)
	{
	case S_PROGRESS:
		if(param > 1.0f)
			param = 1.0f;
		if(param < 0.0f)
			param = 0.0f;
		tbl->SetProgressState(hwnd, TBPF_NORMAL);
		tbl->SetProgressValue(hwnd, (ULONGLONG)(param*100.0f), 100);
		break;
	case S_NO_PROGRESS:		tbl->SetProgressState(hwnd, TBPF_NOPROGRESS);		break;
	case S_WORKING:			tbl->SetProgressState(hwnd, TBPF_INDETERMINATE);	break;
	case S_ERROR:			tbl->SetProgressState(hwnd, TBPF_ERROR);			break;
	case S_PAUSED:			tbl->SetProgressState(hwnd, TBPF_PAUSED);			break;
	}
	last_state = state;
}
