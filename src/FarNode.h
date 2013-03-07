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

#include "Framework/Array.h"
#include "Framework/Node.h"
#include "Framework/CastNode.h"
#include "Framework/ObjectManager.h"

#include "sdk/plugin.hpp"

class FarDlgPayload;
class FarDialogClass;

struct RetCode
{
	int itemNo, retCode;
};

class FarDialog;
class ValueList;

class FarDlgNode : public CastNode<FarDlgNode,FarDlgNode,FarDlgPayload>
{
public:
	FarDlgNode(void);
	virtual ~FarDlgNode(void);

	virtual void InitItem(FarDialogItem& item);
	virtual void RetrieveProperties(HANDLE dlg);
	virtual void BeforeAdd(FarDialogItem& item);
	virtual void LoadState(PropertyMap &state);
	virtual void SaveState(PropertyMap &state);

	virtual void DefSize(int&, int&, int&);

	virtual int IsContainer() { return 0; };
	void AddToItems(Array<FarDialogItem>& Items, Array<RetCode>& RetCodes, int curX, int curY, int curW);
	virtual void ClearDialogItem();
	virtual FarDlgNode* FindChild(const String&);

protected:
	void PreInitItem(FarDialogItem& item);
	//void SetItemText(FarDialogItem& item, const String& text);
	//void DestroyItemText(FarDialogItem& item);

	virtual void BeforeLoad();
	
	
};

class FarDlgContainer : public FarDlgNode
{
public:
	virtual int IsContainer() { return 1; }

protected:
	void AddToItems(Array<FarDialogItem>&, Array<RetCode>&, int, int, int);

	void DefSize(int&, int&, int&);
	void ClearDialogItems(Array<FarDialogItem>&);
	FarDlgNode* FindChild(const String&);
	void LoadState(PropertyMap &state);
	void SaveState(PropertyMap &state);
	void RetrieveProperties(HANDLE dlg);
};

class FarDialog : public FarDlgContainer
{
public:
	FarDialog();
	virtual ~FarDialog();
	int Execute();
	void ResetControls();

	FarDlgNode& operator[](const String&);

	void LoadState(PropertyMap &state) { FarDlgContainer::LoadState(state); }
	void SaveState(PropertyMap &state)	{ FarDlgContainer::SaveState(state); }

protected:
	void BeforeLoad();
};

class FarDialogList : public CastNode<FarDialog,FarDialog,FarDialogClass>
{
};

#endif//__DLGOBJECT_H__
