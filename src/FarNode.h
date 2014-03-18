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
	FarDlgNode();
	virtual ~FarDlgNode(void);

	DEFINE_NODE_CLASS(FarDlgNode);

	virtual void InitItem(FarDialogItem& item);
	virtual void RetrieveProperties(HANDLE dlg);
	virtual void BeforeAdd(FarDialogItem& item);
	virtual void LoadState(PropertyMap &state);
	virtual void SaveState(PropertyMap &state);

	virtual void DefSize(int&, int&, int&);

	virtual int IsContainer() { return 0; }
	virtual void AddToItems(std::vector<FarDialogItem>& Items, std::vector<RetCode>& RetCodes, int curX, int curY, int curW);
	virtual void ClearDialogItem();
	virtual FarDlgNode* FindChild(const String&);

protected:
	virtual void BeforeLoad();
};

class FarDlgContainer : public FarDlgNode
{
public:
	DEFINE_NODE_CLASS(FarDlgContainer);

	virtual int IsContainer() { return 1; }

	virtual void LoadState(PropertyMap &state);
	virtual void SaveState(PropertyMap &state);
protected:
	virtual void AddToItems(std::vector<FarDialogItem>&, std::vector<RetCode>&, int, int, int);

	virtual void DefSize(int&, int&, int&);
	virtual void ClearDialogItems(std::vector<FarDialogItem>&);
	virtual FarDlgNode* FindChild(const String&);

	virtual void RetrieveProperties(HANDLE dlg);
};

class FarDialog : public FarDlgContainer
{
public:
	FarDialog();
	virtual ~FarDialog();

	DEFINE_NODE_CLASS(FarDialog);

	int Execute();
	void ResetControls();

	FarDlgNode& operator[](const String&);

protected:
	void BeforeLoad();
};

class FarDialogList : public CastNode<FarDialog,FarDialog,FarDialogClass>
{
};

#endif//__DLGOBJECT_H__
