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

//#include "../stdhdr.h"
//#include "../valuelist.h"
#include "dlgclass.h"
//#include "plugin.h"
#include "dlgobject.h"
#include "common.h"
#include "Framework/StrUtils.h"
#include "Framework/ObjString.h"
#include "guid.hpp"

FarDlgObject::FarDlgObject(void)
{
	DialogItem=-1;
}

FarDlgObject::~FarDlgObject(void)
{
}

void FarDlgObject::InitItem(FarDialogItem& item)
{ 
	PreInitItem(item);
	Class()->InitItem(item, *this); 
}

void FarDlgObject::RetrieveProperties(HANDLE dlg)
{ 
	Class()->RetrieveProperties(*this, dlg); 
}

void FarDlgObject::BeforeAdd(FarDialogItem& item)
{ 
	Class()->BeforeAdd(item, *this); 
}

void FarDlgObject::LoadState(PropertyMap &state)
{ 
	Class()->LoadState(state, *this); 
}

void FarDlgObject::SaveState(PropertyMap &state)
{ 
	Class()->SaveState(state, *this); 
}

void FarDlgObject::AddToItems(Array<FarDialogItem>& Items, 
							  Array<RetCode>& RetCodes, 
							  int curX, int curY, int curW)
{
	FarDialogItem item;
	memset(&item, 0, sizeof(item));
	item.X1=curX;
	item.Y1=curY;
	item.Y2=curY;
	InitItem(item);
	item.X2=item.X1+curW-1;
	BeforeAdd(item);
	Items.Add(item);
	DialogItem=Items.Count()-1;
	if (item.Type==DI_BUTTON && !(item.Flags & DIF_BTNNOCLOSE))
	{
		int res=Property("Result");
		if (res!=-1)
		{
			RetCode rc;
			rc.itemNo=Items.Count()-1;
			rc.retCode=res;
			RetCodes.Add(rc);
		}
	}
}

void FarDlgObject::SetItemText(FarDialogItem& item, const String& text)
{
	DestroyItemText(item);
	size_t len = text.len() + 1;
	wchar_t* t = new wchar_t[len];
	text.copyTo(t, len);
	item.Data = t;
	item.MaxLength = 0;
}

void FarDlgObject::DestroyItemText(FarDialogItem& item)
{
	delete(item.Data);
	item.Data = NULL;
}

void FarDlgObject::PreInitItem(FarDialogItem& item)
{
	//	SetItemText(&item, Name());

	String p=Property("Text");
	if (p.empty()) {
		String t = Dialog->Name()+"."+Name();
		p=LOC(t);
	}
	SetItemText(item, p);

	for (int i=0; i<AttribCount(); i++)
		if (Property(Attrib(i).Name))
			item.Flags |= Attrib(i).Flag;
	if (Property("Focus")) item.Flags |= DIF_FOCUS;
}

void FarDlgObject::DefSize(int& w, int& h, int& fit)
{
	FarDialogItem item;
	memset(&item, 0, sizeof(item));
	InitItem(item);
	fit=Property("FitWidth");
	w=item.X2-item.X1+1;
	h=item.Y2-item.Y1+1;
	DestroyItemText(item);
}

FarDlgObject* FarDlgObject::FindChild(const String& name)
{
	if (Name()==name) return this;
	else return NULL;
}

struct _group
{
	int start, end, w, h, nfit;
};

void FarDlgContainer::DefSize(int& sumw, int& sumh, int& fit)
{
	sumw=sumh=0;
	int groupw=0, grouph=0;
	fit=Property("FitWidth");
	for (int i=0; i<childs.Count(); i++)
	{
		FarDlgObject &obj=Child(i);
		if (obj.Property("Visible"))
		{
			int w, h, f;
			obj.DefSize(w, h, f);
			groupw+=w+2;
			if (h>grouph) grouph=h;
			if (!obj.Property("NoBreak"))
			{
				if (groupw-2>sumw) sumw=groupw-2;
				sumh+=grouph;
				groupw=grouph=0;
			}
		}
	}
}

void FarDlgContainer::AddToItems(Array<FarDialogItem>& Items, 
								 Array<RetCode>& RetCodes,
								 int curX, int curY, int curW)
{
	int sumw=0, sumh=0;
	Array<_group> Groups;
	_group group;
	group.start=group.w=group.h=group.nfit=0;
	for (int i=0; i<childs.Count(); i++)
	{
		FarDlgObject &obj=Child(i);
		if (obj.Property("Visible")) 
		{
			int w, h, f;
			obj.DefSize(w, h, f);
			if (f) 
			{
				group.nfit++;
				w=0;
			} 
			group.w+=w+2;
			if (h>group.h) group.h=h;
			if (!obj.Property("NoBreak"))
			{
				group.w-=2;
				if (group.w>sumw) sumw=group.w;
				sumh+=group.h;
				group.end=i;
				Groups.Add(group);
				group.w=group.h=group.nfit=0;
				group.start=i+1;
			}
		}
	}
	int x=curX, y=curY;
	for (int j=0; j<Groups.Count(); j++)
	{
		for (int i=Groups[j].start; i<=Groups[j].end; i++)
		{
			FarDlgObject &obj=Child(i);
			if (obj.Property("Visible"))
			{
				int w, h, f;
				obj.DefSize(w, h, f);
				if (f) { 
					w=(curW-Groups[j].w)/Groups[j].nfit;
				}
				obj.AddToItems(Items, RetCodes, x, y, w);
				x+=w+2;
			}
		}
		x=curX;
		y+=Groups[j].h;
	}
}

void FarDlgContainer::LoadState(PropertyMap& state)
{
	for (int i=0; i<childs.Count(); i++)
		if (Child(i).IsContainer() || (bool)Child(i)("Persistent"))
			Child(i).LoadState(state);
}

void FarDlgContainer::SaveState(PropertyMap& state)
{
	for (int i=0; i<childs.Count(); i++)
		if (Child(i).IsContainer() || (bool)Child(i)("Persistent"))
			Child(i).SaveState(state);
}

void FarDlgContainer::RetrieveProperties(HANDLE dlg)
{
	int cnt = childs.Count();
	for (int i=0; i<cnt; i++) {
		FarDlgObject& fdo = Child(i);
		if (fdo.DialogItem!=-1 || fdo.IsContainer())
			fdo.RetrieveProperties(dlg);
	}
}

void FarDlgContainer::ClearDialogItems(Array<FarDialogItem>& Items)
{
	for(int i = 0; i < Items.Count(); ++i)
	{
		DestroyItemText(Items[i]);
	}
	for (int i=0; i<childs.Count(); i++)
		Child(i).ClearDialogItem();
}

FarDlgObject* FarDlgContainer::FindChild(const String& name)
{
	if (Name()==name) return this;
	for (int i=0; i<childs.Count(); i++)
	{
		FarDlgObject* obj=Child(i).FindChild(name);
		if (obj) return obj;
	}
	return NULL;
}

void FarDlgObject::BeforeLoad()
{
	Dialog=Parent()->Dialog;
}

// ===== FarDialog:: =====
FarDialog::FarDialog(void)
{
}

FarDialog::~FarDialog(void)
{
}

int FarDialog::Execute()
{
  Array<FarDialogItem> Items;
  Array<RetCode> RetCodes;
  FarDialogItem frame;
  memset(&frame, 0, sizeof(frame));
  frame.Type=DI_DOUBLEBOX;
  String p=Property("Title");
  if (p.empty()) p=LOC(Name());
  SetItemText(frame, p);
  Items.Add(frame);

  int w, h, f;
  DefSize(w, h, f);
  AddToItems(Items, RetCodes, 5, 2, w);

  Items[0].X1=3; Items[0].Y1=1; 
  Items[0].X2=w+6; Items[0].Y2=h+2;

	HANDLE hnd = Info.DialogInit(&MainGuid, &MainDialog, -1, -1, w+10, h+4, 
		String(Property("HelpTopic")).c_str(), 
		(FarDialogItem*)Items.Storage(), Items.Count(),
		0, bool(Property("Warning")) ? FDLG_WARNING : 0,
		Info.DefDlgProc, 0
	);  // !!! Need real Dialog GUID, instead of MainDialog
	int ret=-1;
	if (hnd!=INVALID_HANDLE_VALUE) {
	int res = Info.DialogRun(hnd);
    for (int i=0; i<RetCodes.Count(); i++) {
		if (RetCodes[i].itemNo == res) {
			if (RetCodes[i].retCode!=-1) {
				RetrieveProperties(hnd);
			}
			ret=RetCodes[i].retCode;
			break;
		}
	}
  }
  ClearDialogItems(Items);
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

