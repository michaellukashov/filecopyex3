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

#include "FarNode.h"
#include "FarPayload.h"
#include "common.h"
#include "Framework/Node.h"
#include "Framework/ObjectManager.h"
#include "Framework/StringVector.h"

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

void DestroyItemText(FarDialogItem& item)
{
	delete(item.Data);
	item.Data = NULL;
}

void SetItemText(FarDialogItem& item, const String& text)
{
	DestroyItemText(item);
	size_t len = text.len() + 1;
	wchar_t* t = new wchar_t[len];
	text.copyTo(t, len);
	item.Data = t;
	item.MaxLength = 0;
}

// ===== FarDlgPayload =====
FarDlgPayload::FarDlgPayload()
{
	dialog = NULL;
	dialogItem = -1;
}

FarDlgPayload::~FarDlgPayload()
{
}

void FarDlgPayload::init(const String &_name)
{
	Payload::init(_name);

	for (int i=0; i<AttribCount(); i++) {
		addProperty(Attrib(i).Name, 0);
	}
	addProperty("FitWidth", 0);
	addProperty("Focus", 0);
	addProperty("NoBreak", 0);
	addProperty("Visible", 1);
	addProperty("Persistent", 0);
	addProperty("Text", "");
}

void FarDlgPayload::preInitItem(FarDialogItem& item)
{
	//	SetItemText(&item, Name());

	String p = getProp("Text");
	if (p.empty()) {
		String name = getDialog()->getName() + "." + getName();
		String loc = LOC(name);
		p = (name == loc) ? "": loc;
	}
	SetItemText(item, p);

	for (int i=0; i<AttribCount(); i++) {
		if (getProp(Attrib(i).Name)) {
			item.Flags |= Attrib(i).Flag;
		}
	}
	if (getProp("Focus")) {
		item.Flags |= DIF_FOCUS;
	}
}

void FarDlgPayload::realInitItem(FarDialogItem& item )
{
}

void FarDlgPayload::InitItem(FarDialogItem& item )
{
	preInitItem(item);
	realInitItem(item);
}

void FarDlgPayload::DefSize(int& w, int& h, int& fit)
{
	FarDialogItem item;
	memset(&item, 0, sizeof(item));
	InitItem(item);
	fit=getProp("FitWidth");
	w=item.X2-item.X1+1;
	h=item.Y2-item.Y1+1;
	DestroyItemText(item);
}

void FarDlgPayload::AddToItems(std::vector<FarDialogItem>& Items, std::vector<RetCode>& RetCodes, int curX, int curY, int curW)
{
	FarDialogItem item;
	memset(&item, 0, sizeof(item));
	item.X1 = curX;
	item.Y1 = curY;
	item.Y2 = curY;
	InitItem(item);
	item.X2 = item.X1 + curW - 1;
	BeforeAdd(item);
	Items.push_back(item);
	dialogItem = Items.size()-1;
	if (item.Type == DI_BUTTON && !(item.Flags & DIF_BTNNOCLOSE)) {
		int res = getProp("Result");
		if (res != -1) {
			RetCode rc;
			rc.itemNo = Items.size()-1;
			rc.retCode = res;
			RetCodes.push_back(rc);
		}
	}
}

// FarDlgCheckboxPayload
void FarDlgCheckboxPayload::RetrieveProperties(HANDLE dlg)
{
	getProp("Selected") = (int)Info.SendDlgMessage(dlg, DM_GETCHECK, dialogItem, 0);
}

// FarDlgEditPayload
void FarDlgEditPayload::realInitItem(FarDialogItem& item)
{
	item.Type=DI_EDIT;
	int w = getProp("Width");
	item.X2 = item.X1 + w - 1;
	if (item.Flags & DIF_HISTORY) {
		String p = getProp("HistoryId");
		if (!p.empty()) {
			(String("FarFramework\\") + getDialog()->getName() + "\\" + getName()).copyTo(HistoryId, sizeof(HistoryId)/sizeof(wchar_t));
			item.History = HistoryId;
		}
	}
}

static String GetDlgText(HANDLE dlg, int id)
{
	FarDialogItemData item = { sizeof(FarDialogItemData) };
	item.PtrLength = Info.SendDlgMessage(dlg, DM_GETTEXT, id, NULL);
	item.PtrData = new wchar_t[item.PtrLength+1];
	Info.SendDlgMessage(dlg, DM_GETTEXT, id, &item);
	String t(item.PtrData);
	delete(item.PtrData);
	return t;
}

void FarDlgEditPayload::RetrieveProperties(HANDLE dlg)
{
	getProp("Text") = GetDlgText(dlg, dialogItem);
}

// FarDlgComboboxPayload
void FarDlgComboboxPayload::realInitItem(FarDialogItem& item)
{
	item.Type=DI_COMBOBOX;
	int w = getProp("Width");
	item.X2=item.X1+w-1;

	item.ListItems = &list;
	if (list.Items) {
		delete(list.Items);
		list.Items = NULL;
		list.ItemsNumber = 0;
	}
	StringVector items;
	items.loadFromString(getProp("Items"), '\n');
	if (items.Count()) {
		list.ItemsNumber = items.Count();
		list.Items = new FarListItem[items.Count()];
		for (int i=0; i<items.Count(); i++) {
			if (getProp("Text") == items[i]) {
				list.Items[i].Flags |= LIF_SELECTED;
			}
		}
	}
}

void FarDlgComboboxPayload::RetrieveProperties(HANDLE dlg)
{
	getProp("Text") = GetDlgText(dlg, dialogItem);
}

size_t lablen(FarDialogItem& item)
{
	if (item.Flags & DIF_SHOWAMPERSAND) {
		return wcslen(item.Data);
	} else {
			int res=0;
			for (const wchar_t *p=item.Data; *p; p++) {
			if (*p!='&') res++;
		}
		return res;
	}
}

void InitObjMgr()
{
	objectManager = new ObjectManager;
	objectManager->regClass("FarDlgLine", &FarDlgLinePayload::create, &FarDlgNode::create);
	objectManager->regClass("FarDlgLabel", &FarDlgLabelPayload::create, &FarDlgNode::create);
	objectManager->regClass("FarDlgButton", &FarDlgButtonPayload::create, &FarDlgNode::create);
	objectManager->regClass("FarDlgCheckbox", &FarDlgCheckboxPayload::create, &FarDlgNode::create);
	objectManager->regClass("FarDlgRadioButton", &FarDlgRadioButtonPayload::create, &FarDlgNode::create);
	objectManager->regClass("FarDlgEdit", &FarDlgEditPayload::create, &FarDlgNode::create);
	objectManager->regClass("FarDlgCombobox", &FarDlgComboboxPayload::create, &FarDlgNode::create);

	objectManager->regClass("FarDlgPanel", &FarDlgPanelPayload::create, &FarDlgContainer::create);

	objectManager->regClass("FarDialog", &FarDialogPayload::create, &FarDialog::create);

}

void DoneObjMgr()
{
	delete objectManager;
}
