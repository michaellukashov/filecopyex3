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
#include "interface/plugin.hpp"
#include "dlgobject.h"
#include "dlgclass.h"
#include "plugin.h"

static const Attribute _Attrib[] =
{
//	{ "SetColor", DIF_SETCOLOR },
	{ "BoxColor", DIF_BOXCOLOR }, 
	{ "Group", DIF_GROUP }, 
	{ "LeftText", DIF_LEFTTEXT }, 
	{ "MoveSelect", DIF_MOVESELECT }, 
	{ "ShowAmpersand", DIF_SHOWAMPERSAND }, 
	{ "CenterGroup", DIF_CENTERGROUP }, 
	{ "NoBrackets", DIF_NOBRACKETS }, 
	{ "Separator", DIF_SEPARATOR }, 
	{ "Editor", DIF_EDITOR }, 
	{ "History", DIF_HISTORY }, 
	{ "EditExpand", DIF_EDITEXPAND }, 
	{ "DropdownList", DIF_DROPDOWNLIST }, 
	{ "UseLastHistory", DIF_USELASTHISTORY }, 
	{ "BtnNoClose", DIF_BTNNOCLOSE }, 
	{ "SelectOnEntry", DIF_SELECTONENTRY }, 
	{ "NoFocus", DIF_NOFOCUS }, 
	{ "MaskEdit", DIF_MASKEDIT }, 
	{ "Disable", DIF_DISABLE }, 
	{ "ListNoAmpersand", DIF_LISTNOAMPERSAND }, 
	{ "ReadOnly", DIF_READONLY }, 
	{ "3State", DIF_3STATE }, 
//	{ "VarEdit", DIF_VAREDIT }, 
	{ "Hidden", DIF_HIDDEN }, 
	{ "ManualAddHistory", DIF_MANUALADDHISTORY }, 
};
const Attribute& Attrib(int i) { return _Attrib[i]; }
int AttribCount() { return sizeof(_Attrib)/sizeof(Attribute); }

void FarDlgObjectClass::DefineProperties()
{
  for (int i=0; i<AttribCount(); i++)
    AddProperty(Attrib(i).Name, 0);
  AddProperty("FitWidth", 0);
  AddProperty("Focus", 0);
  AddProperty("NoBreak", 0);
  AddProperty("Visible", 1);
  AddProperty("Persistent", 0);
  AddProperty("Text", "");
}

void FarDlgCheckboxClass::RetrieveProperties(FarDlgObject& obj, HANDLE dlg)
{
	obj("Selected")=(int)Info.SendDlgMessage(dlg, DM_GETCHECK, obj.DialogItem, 0);
}

void FarDlgEditClass::InitItem(FarDialogItem& item, FarDlgObject& obj)
{
  item.Type=DI_EDIT;
  int w=obj("Width");
  item.X2=item.X1+w-1;
  if (item.Flags & DIF_HISTORY)
  {
    String p=obj("HistoryId");
    if (p!="") 
    {
      FarDlgEdit &e=static_cast<FarDlgEdit&>(obj);
      (String("FarFramework\\")+e.GetDialog()->Name()+"\\"+e.Name()).ToUnicode(e.HistoryId, sizeof(e.HistoryId)/sizeof(wchar_t));
      item.History=e.HistoryId;
    }
  }
}

static String GetDlgText(HANDLE dlg, int id)
{
	size_t len = Info.SendDlgMessage(dlg, DM_GETTEXTPTR, id, NULL);
	wchar_t* buf = (wchar_t*)malloc((len + 1)*sizeof(wchar_t));
	Info.SendDlgMessage(dlg, DM_GETTEXTPTR, id, (LONG_PTR)buf);
	String t(buf);
	free(buf);
	return t;
}

void FarDlgEditClass::RetrieveProperties(FarDlgObject& obj, HANDLE dlg)
{
	obj("Text")=GetDlgText(dlg, obj.DialogItem);
}

void FarDlgComboboxClass::InitItem(FarDialogItem& item, FarDlgObject& obj)
{
  item.Type=DI_COMBOBOX;
  int w=obj("Width");
  item.X2=item.X1+w-1;

  FarDlgCombobox &e=static_cast<FarDlgCombobox&>(obj);
  item.ListItems=&e.list;
  if (e.list.Items) 
  {
    free(e.list.Items);
    e.list.Items=NULL;
    e.list.ItemsNumber=0;
  }
  StringList items;
  items.LoadFromString(obj("Items"), '\n');
  if (items.Count())
  {
    e.list.ItemsNumber=items.Count();
    int sz=sizeof(FarListItem)*items.Count();
    e.list.Items=(FarListItem*)malloc(sz);
    memset(e.list.Items, 0, sz);
    for (int i=0; i<items.Count(); i++)
    {
//      strncpy_s(e.list.Items[i].Text, 128, items[i].optr(), sizeof(e.list.Items[i].Text));
      if (obj("Text")==items[i])
        e.list.Items[i].Flags|=LIF_SELECTED;
    }
  }
}
void FarDlgComboboxClass::RetrieveProperties(FarDlgObject& obj, HANDLE dlg)
{
	obj("Text")=GetDlgText(dlg, obj.DialogItem);
}

int lablen(FarDialogItem& item)
{
  if (item.Flags & DIF_SHOWAMPERSAND)
    return (int)wcslen(item.PtrData);
  else
  {
    int res=0;
    for (const wchar_t *p=item.PtrData; *p; p++)
      if (*p!='&') res++;
    return res;
  }
}

void InitObjMgr()
{
	objectManager = new ObjectManager;
	objectManager->RegisterClass(new FarDlgLineClass);
	objectManager->RegisterClass(new FarDlgLabelClass);
	objectManager->RegisterClass(new FarDlgPanelClass);
	objectManager->RegisterClass(new FarDlgButtonClass);
	objectManager->RegisterClass(new FarDlgCheckboxClass);
	objectManager->RegisterClass(new FarDlgRadioButtonClass);
	objectManager->RegisterClass(new FarDlgEditClass);
	objectManager->RegisterClass(new FarDlgComboboxClass);
	objectManager->RegisterClass(new FarDialogClass);
}
void DoneObjMgr()
{
	delete objectManager;
}
