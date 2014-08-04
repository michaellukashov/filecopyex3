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

#include "Framework/StrUtils.h"
#include "Framework/ObjString.h"
#include "FarNode.h"
#include "FarPayload.h"
#include "Common.h"
#include "guid.hpp"


// FarDlgNode
FarDlgNode::FarDlgNode(void)
{
}

FarDlgNode::~FarDlgNode(void)
{
}

void FarDlgNode::InitItem(FarDialogItem & item)
{
  getPayload().InitItem(item);
}

void FarDlgNode::RetrieveProperties(HANDLE dlg)
{
  getPayload().RetrieveProperties(dlg);
}

void FarDlgNode::BeforeAdd(FarDialogItem & item)
{
  getPayload().BeforeAdd(item);
}

void FarDlgNode::LoadState(PropertyMap & state)
{
  getPayload().LoadState(state);
}

void FarDlgNode::SaveState(PropertyMap & state)
{
  getPayload().SaveState(state);
}

void FarDlgNode::AddToItems(std::vector<FarDialogItem>& Items, std::vector<RetCode>& RetCodes, intptr_t curX, intptr_t curY, intptr_t curW)
{
  getPayload().AddToItems(Items, RetCodes, curX, curY, curW);
}

void FarDlgNode::DefSize(intptr_t & w, intptr_t & h, intptr_t & fit)
{
  getPayload().DefSize(w, h, fit);
}

void FarDlgNode::ClearDialogItem()
{
  getPayload().ClearDialogItem();
}

FarDlgNode * FarDlgNode::FindChild(const String & name)
{
  if (getName() == name)
    return this;
  else
    return nullptr;
}

struct _group
{
  intptr_t start, end, w, h, nfit;
};

void FarDlgContainer::DefSize(intptr_t & sumw, intptr_t & sumh, intptr_t & fit)
{
  sumw = sumh = 0;
  intptr_t groupw = 0, grouph = 0;
  fit = getPayload()(L"FitWidth").operator int();
  for (size_t i = 0; i < childs.size(); i++)
  {
    FarDlgNode & obj = child(i);
    if (obj(L"Visible"))
    {
      intptr_t w, h, f;
      obj.DefSize(w, h, f);
      groupw += w + 2;
      if (h > grouph)
        grouph = h;
      if (!obj(L"NoBreak"))
      {
        if (groupw - 2 > sumw)
          sumw = groupw - 2;
        sumh += grouph;
        groupw = grouph = 0;
      }
    }
  }
}

void FarDlgContainer::AddToItems(std::vector<FarDialogItem>& Items, std::vector<RetCode>& RetCodes, intptr_t curX, intptr_t curY, intptr_t curW)
{
  intptr_t sumw = 0, sumh = 0;
  std::vector<_group> Groups;
  _group group;
  group.start = group.w = group.h = group.nfit = 0;
  for (size_t i = 0; i < childs.size(); i++)
  {
    FarDlgNode & obj = child(i);
    if (obj(L"Visible"))
    {
      intptr_t w, h, f;
      obj.DefSize(w, h, f);
      if (f)
      {
        group.nfit++;
        w = 0;
      }
      group.w += w + 2;
      if (h > group.h)
        group.h = h;
      if (!obj(L"NoBreak"))
      {
        group.w -= 2;
        if (group.w > sumw)
          sumw = group.w;
        // sumh+=group.h;
        group.end = i;
        Groups.push_back(group);
        group.w = group.h = group.nfit = 0;
        group.start = i + 1;
      }
    }
  }
  intptr_t x = curX, y = curY;
  for (size_t j = 0; j < Groups.size(); j++)
  {
    for (intptr_t i = Groups[j].start; i <= Groups[j].end; i++)
    {
      FarDlgNode & obj = child(i);
      if (obj(L"Visible"))
      {
        intptr_t w, h, f;
        obj.DefSize(w, h, f);
        if (f)
        {
          w = (curW - Groups[j].w) / Groups[j].nfit;
        }
        obj.AddToItems(Items, RetCodes, x, y, w);
        x += w + 2;
      }
    }
    x = curX;
    y += Groups[j].h;
  }
}

void FarDlgContainer::LoadState(PropertyMap & state)
{
  for (size_t i = 0; i < childs.size(); i++)
  {
    if (child(i).IsContainer() || (bool)child(i)(L"Persistent"))
    {
      child(i).LoadState(state);
    }
  }
}

void FarDlgContainer::SaveState(PropertyMap & state)
{
  for (size_t i = 0; i < childs.size(); i++)
    if (child(i).IsContainer() || (bool)child(i)(L"Persistent"))
      child(i).SaveState(state);
}

void FarDlgContainer::RetrieveProperties(HANDLE dlg)
{
  size_t cnt = childs.size();
  for (size_t i = 0; i < cnt; i++)
  {
    FarDlgNode & fdo = child(i);
    if (fdo.getPayload().getDialogItem() != -1 || fdo.IsContainer())
      fdo.RetrieveProperties(dlg);
  }
}

void FarDlgContainer::ClearDialogItems(std::vector<FarDialogItem>& Items)
{
  for (size_t i = 0; i < Items.size(); i++)
  {
    DestroyItemText(Items[i]);
  }
  for (size_t i = 0; i < childs.size(); i++)
  {
    child(i).ClearDialogItem();
  }
}

FarDlgNode * FarDlgContainer::FindChild(const String & name)
{
  if (getName() == name)
    return this;
  for (size_t i = 0; i < childs.size(); i++)
  {
    FarDlgNode * obj = child(i).FindChild(name);
    if (obj)
      return obj;
  }
  return nullptr;
}

void FarDlgNode::BeforeLoad()
{
  getPayload().setDialog(Parent()->getPayload().getDialog());
}

// ===== FarDialog:: =====
FarDialog::FarDialog()
{
}

FarDialog::~FarDialog()
{
}

intptr_t FarDialog::Execute()
{
  std::vector<FarDialogItem> Items;
  std::vector<RetCode> RetCodes;
  FarDialogItem frame;
  memset(&frame, 0, sizeof(frame));
  frame.Type = DI_DOUBLEBOX;
  String p = (*this)(L"Title");
  if (p.empty())
  {
    p = LOC(getName());
  }
  SetItemText(frame, p);
  Items.push_back(frame);

  intptr_t w, h, f;
  DefSize(w, h, f);
  AddToItems(Items, RetCodes, 5, 2, w);

  Items[0].X1 = 3;
  Items[0].Y1 = 1;
  Items[0].X2 = w + 6;
  Items[0].Y2 = h + 2;
  String HelpTopic = Property((*this)("HelpTopic"));

  HANDLE hnd = Info.DialogInit(&MainGuid, &MainDialog, -1, -1, w + 10, h + 4,
                               HelpTopic.c_str(),
                               Items.data(), Items.size(),
                               0, bool((*this)(L"Warning")) ? FDLG_WARNING : 0,
                               Info.DefDlgProc, 0
                              );  // !!! Need real Dialog GUID, instead of MainDialog
  intptr_t ret = -1;
  if (hnd != INVALID_HANDLE_VALUE)
  {
    intptr_t res = Info.DialogRun(hnd);
    for (size_t i = 0; i < RetCodes.size(); i++)
    {
      if (RetCodes[i].itemNo == res)
      {
        if (RetCodes[i].retCode != -1)
        {
          RetrieveProperties(hnd);
        }
        ret = RetCodes[i].retCode;
        break;
      }
    }
    Info.DialogFree(hnd);
  }
  ClearDialogItems(Items);
  return ret;
}

void FarDialog::BeforeLoad()
{
  getPayload().setDialog(this);
}

void FarDialog::ResetControls()
{
  ReloadPropertiesRecursive();
}

FarDlgNode & FarDialog::operator[](const String & n)
{
  FarDlgNode * obj = FindChild(n);
  if (!obj)
    FWError(Format(L"Request to undefined object %s", n.ptr()));
  return *obj;
}
