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

#ifndef	__DLGOBJECT_H__
#define	__DLGOBJECT_H__

#pragma once

#include "../array.h"
#include "../object.h"
#include "interface/plugin.hpp"

class FarDlgObjectClass;
class FarDialogClass;

struct RetCode
{
	int itemNo, retCode;
};

class FarDialog;
class ValueList;

class FarDlgObject : public CastObject<FarDlgObject, FarDlgObject, FarDlgObjectClass>
{
public:
	FarDlgObject(void);
	virtual ~FarDlgObject(void);

	virtual void InitItem(FarDialogItem& item);
	virtual void RetrieveProperties(Array<FarDialogItem>& items, HANDLE dlg);
	virtual void BeforeAdd(FarDialogItem& item);
	virtual void LoadState(PropertyList &state);
	virtual void SaveState(PropertyList &state);
	int DialogItem;

	FarDialog* GetDialog() { return Dialog; }

protected:
	virtual void AddToItems(Array<FarDialogItem>&, Array<RetCode>&, int, int, int);
	void PreInitItem(FarDialogItem& item);

	FarDialog *Dialog;
	virtual void BeforeLoad();
	virtual void DefSize(int&, int&, int&);
	virtual void ClearDialogItem() { DialogItem=-1; }
	virtual FarDlgObject* FindChild(const String&);

	virtual int IsContainer() { return 0; }

	friend class FarDlgContainer;
};

class FarDlgEdit : public FarDlgObject
{
public:
	wchar_t HistoryId[64];
};

class FarDlgCombobox : public FarDlgObject
{
public:
	FarList list;
	FarDlgCombobox(void) { list.ItemsNumber=0; list.Items=NULL; }
	virtual ~FarDlgCombobox(void) { if (list.Items) free(list.Items); }
};

class FarDlgContainer : public FarDlgObject
{
protected:
	void AddToItems(Array<FarDialogItem>&, Array<RetCode>&, int, int, int);

	void DefSize(int&, int&, int&);
	void ClearDialogItem();
	FarDlgObject* FindChild(const String&);
	void LoadState(PropertyList &state);
	void SaveState(PropertyList &state);
	void RetrieveProperties(Array<FarDialogItem>& items, HANDLE dlg);

	int IsContainer() { return 1; }
};

class FarDialog : public FarDlgContainer
{
public:
	FarDialog();
	virtual ~FarDialog();
	int Execute();
	void ResetControls();

	FarDlgObject& operator[](const String&);

	void LoadState(PropertyList &state) { FarDlgContainer::LoadState(state); }
	void SaveState(PropertyList &state)	{ FarDlgContainer::SaveState(state); }

protected:
	void BeforeLoad();
};

class FarDialogList : public CastObject<FarDialog, FarDialogList, FarDialogClass>
{
};

#endif//__DLGOBJECT_H__
