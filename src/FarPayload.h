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

#ifndef	__DLGCLASS_H__
#define	__DLGCLASS_H__

#pragma once

#include "Framework/Node.h"
#include "SDK/plugin.hpp"
#include "FarPayload.h"

class FarDlgNode;
class FarDialog;

struct Attribute
{
	const char* Name;
	uint64_t Flag;
};
const Attribute& Attrib(int i);
int AttribCount();

size_t lablen(FarDialogItem& item);
String FormatWidth(const String& v, int len);

void SetItemText(FarDialogItem& item, const String& text);
void DestroyItemText(FarDialogItem& item);

// ===== FarDlgPayload =====
class FarDlgPayload: public Payload
{
public:
	FarDlgPayload();
	~FarDlgPayload();

	virtual void init(const String &_name);

	virtual void InitItem(FarDialogItem& item );
	virtual void RetrieveProperties(HANDLE) { ; }
	virtual void BeforeAdd(FarDialogItem&) { ; }
	virtual void LoadState(PropertyMap &state) { ; }
	virtual void SaveState(PropertyMap &state) { ; }

	virtual void DefSize(int&, int&, int&);
	virtual void ClearDialogItem() { dialogItem = -1; }

	FarDialog* getDialog() { return dialog; }
	void setDialog(FarDialog *_dialog) { dialog = _dialog; }
	int getDialogItem() { return dialogItem; }

	void AddToItems(std::vector<FarDialogItem>& Items, std::vector<RetCode>& RetCodes, int curX, int curY, int curW);

protected:
	virtual void preInitItem(FarDialogItem& item);
	virtual void realInitItem(FarDialogItem& item);

	FarDialog *dialog;
	int dialogItem; // should be int, we are using -1
};

// ===== FarDlgLinePayload =====
class FarDlgLinePayload : public FarDlgPayload
{
public:
	DEFINE_CLASS("FarDlgLine", FarDlgLinePayload)

	void realInitItem(FarDialogItem& item)
	{
		item.Type=DI_TEXT;
		item.Flags|=DIF_SEPARATOR | DIF_BOXCOLOR;
	}
};

// ===== FarDlgLabelPayload =====
class FarDlgLabelPayload : public FarDlgPayload
{
public:
	DEFINE_CLASS("FarDlgLabel", FarDlgLabelPayload)

protected:
	virtual void init(const String &_name)
	{
		FarDlgPayload::init(_name);
		addProperty("Shorten", 0);
	}

	void realInitItem(FarDialogItem& item)
	{
		item.Type=DI_TEXT;
		item.X2 = getProp("Shorten") ? (item.X1-1) : (item.X1+lablen(item)-1);
	}

	void BeforeAdd(FarDialogItem& item, FarDlgNode& obj)
	{
//		if (obj("Shorten"))
//			FormatWidth(item.PtrData, __min(item.X2-item.X1+1, 500))
//			.ToAnsi(item.PtrData, sizeof(item.PtrData));
	}
};

// ===== FarDlgButtonPayload =====
class FarDlgButtonPayload : public FarDlgPayload
{
public:
	DEFINE_CLASS("FarDlgButton", FarDlgButtonPayload)

protected:
	virtual void init(const String &_name)
	{
		FarDlgPayload::init(_name);
		addProperty("Default", 0);
		addProperty("Result", -1);
	}
	virtual void realInitItem(FarDialogItem& item)
	{
		item.Type = DI_BUTTON;
		item.X2 = item.X1 + lablen(item) + 4 - 1;
		if (getProp("Default")) {
			item.Flags |= DIF_DEFAULTBUTTON;
		}
	}
};


// ====== FarDlgPanelPayload ======
class FarDlgPanelPayload: public FarDlgPayload
{
public:
	DEFINE_CLASS("FarDlgPanel", FarDlgPanelPayload)

};


// FarDlgCheckboxPayload
class FarDlgCheckboxPayload: public FarDlgPayload
{
public:
	DEFINE_CLASS("FarDlgCheckbox", FarDlgCheckboxPayload)

protected:
	virtual void init(const String &_name)
	{
		FarDlgPayload::init(_name);
		addProperty("Selected", 0);
	}

	void realInitItem(FarDialogItem& item)
	{
		item.Type=DI_CHECKBOX;
		item.X2=item.X1+lablen(item)+4-1;
		item.Selected=int(getProp("Selected"));
	}

	void RetrieveProperties(HANDLE dlg);

	void LoadState(PropertyMap &state)
	{
		getProp("Selected") = state[getName()];
	}
	void SaveState(PropertyMap &state)
	{
		state[getName()] = getProp("Selected");
	}
};


// FarDlgRadioButtonPayload
class FarDlgRadioButtonPayload: public FarDlgCheckboxPayload
{
public:
	DEFINE_CLASS("FarDlgRadioButton", FarDlgRadioButtonPayload)

protected:

	void realInitItem(FarDialogItem& item)
	{
		FarDlgCheckboxPayload::realInitItem(item);

		item.Type=DI_RADIOBUTTON;
	}
};

// FarDlgEditPayload
class FarDlgEditPayload: public FarDlgPayload
{
public:
	DEFINE_CLASS("FarDlgEdit", FarDlgEditPayload)

protected:
	wchar_t HistoryId[64];

	virtual void init(const String &_name)
	{
		FarDlgPayload::init(_name);
		addProperty("HistoryId", "FarFramework\\DefaultEditHistory");
		addProperty("Width", 10);
	}

	void realInitItem(FarDialogItem& item);
	void RetrieveProperties(HANDLE dlg);
	void LoadState(PropertyMap &state)
	{
		getProp("Text") = state[getName()];
	}

	void SaveState(PropertyMap &state)
	{
		state[getName()] = getProp("Text");
	}

	void BeforeAdd(FarDialogItem& item)
	{
		if (getProp("History")) {
			item.X2--;
		}
	}
};

// FarDlgComboboxPayload
class FarDlgComboboxPayload: public FarDlgPayload
{
public:
	DEFINE_CLASS("FarDlgCombobox", FarDlgComboboxPayload)

	FarDlgComboboxPayload() { list.ItemsNumber=0; list.Items=NULL; }
	virtual ~FarDlgComboboxPayload() { if (list.Items) delete(list.Items); }

protected:
	FarList list;

	virtual void init(const String &_name)
	{
		FarDlgPayload::init(_name);
		addProperty("Width", 10);
		addProperty("Items", "");
	}

	void realInitItem(FarDialogItem& item);
	void RetrieveProperties(HANDLE dlg);
	void LoadState(PropertyMap &state)
	{
		getProp("Text") = state[getName()];
	}
	void SaveState(PropertyMap &state, FarDlgNode& obj)
	{
		state[getName()] = getProp("Text");
	}
	void BeforeAdd(FarDialogItem& item, FarDlgNode& obj)
	{
		item.X2--;
	}
};

// FarDialogPayload
class FarDialogPayload : public FarDlgPayload
{
public:
	DEFINE_CLASS("FarDialog", FarDialogPayload)

protected:
	virtual void init(const String &_name)
	{
		FarDlgPayload::init(_name);
		addProperty("HelpTopic", "NoTopic");
		addProperty("Title", "");
		addProperty("Warning", 0);
	}
};

void InitObjMgr();
void DoneObjMgr();

#endif//__DLGCLASS_H__
