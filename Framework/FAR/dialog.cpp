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

#include "../stdhdr.h"
#include "../common.h"
#include "../object.h"
#include "../array.h"
#include "../strutils.h"
#include "plugin.h"

extern PluginStartupInfo Info;

FarDialog::FarDialog(void)
{
}

FarDialog::~FarDialog(void)
{
}

void SetItemText(FarDialogItem* item, const String& text);

int FarDialog::Execute()
{
  Array<FarDialogItem> Items;
  Array<RetCode> RetCodes;
  FarDialogItem frame;
  memset(&frame, 0, sizeof(frame));
  frame.Type=DI_DOUBLEBOX;
  String p=Property("Title");
  if (p=="") p=LOC(Name());
  SetItemText(&frame, p);
  Items.Add(frame);

  int w, h, f;
  DefSize(w, h, f);
  AddToItems(Items, RetCodes, 5, 2, w);

  Items[0].X1=3; Items[0].Y1=1; 
  Items[0].X2=w+6; Items[0].Y2=h+2;

  HANDLE hnd=Info.DialogInit(Info.ModuleNumber, -1, -1, w+10, h+4, 
    String(Property("HelpTopic")).ptr(), 
    (FarDialogItem*)Items.Storage(), Items.Count(),
    0, bool(Property("Warning")) ? FDLG_WARNING : 0,
    Info.DefDlgProc, 0);
  int ret=-1;
  if (hnd!=INVALID_HANDLE_VALUE)
  {
	  int res = Info.DialogRun(hnd);
    for (int i=0; i<RetCodes.Count(); i++)
      if (RetCodes[i].itemNo==res)
      {
        if (RetCodes[i].retCode!=-1)
          RetrieveProperties(Items, hnd);
        ret=RetCodes[i].retCode;
        break;
      }
  }
  ClearDialogItem();
  return ret;
}

void FarDialog::BeforeLoad()
{
  Dialog=this;
}

void FarDialog::ResetControls()
{
  ReloadPropertiesRecursive();
}

FarDlgObject& FarDialog::operator[](const String& n)
{
  FarDlgObject *obj=FindChild(n);
  if (!obj)
    FWError(Format(L"Request to undefined object %s", n.ptr()));
  return *obj;
}

