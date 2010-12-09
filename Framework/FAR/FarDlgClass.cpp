#include <stdhdr.h>
#include "far/interface/plugin.hpp"
#include "far/fardlgobject.h"
#include "far/fardlgclass.h"
#include "farplugin.h"

void FarDlgObjectClass::DefineProperties()
{
  for (int i=0; i<sizeof(Attrib)/sizeof(Attrib[0]); i++)
    AddProperty(Attrib[i].Name, 0);
  AddProperty("FitWidth", 0);
  AddProperty("Focus", 0);
  AddProperty("NoBreak", 0);
  AddProperty("Visible", 1);
  AddProperty("Persistent", 0);
  AddProperty("Text", "");
}

FarDlgObjectReg *__FarDlgObjectReg;

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
void FarDlgEditClass::RetrieveProperties(Array<FarDialogItem>& items, FarDlgObject& obj, HANDLE dlg)
{
	size_t len = Info.SendDlgMessage(dlg, DM_GETTEXTPTR, obj.DialogItem, NULL);
	wchar_t* buf = (wchar_t*)_malloca((len + 1)*sizeof(wchar_t));
	Info.SendDlgMessage(dlg, DM_GETTEXTPTR, obj.DialogItem, (LONG_PTR)buf);
//	_toansi(items[obj.DialogItem].PtrData);
	obj("Text")=buf;
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
