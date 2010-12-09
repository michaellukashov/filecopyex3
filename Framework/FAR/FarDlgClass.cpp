#include <stdhdr.h>
#include "far/interface/plugin.hpp"
#include "far/fardlgobject.h"
#include "far/fardlgclass.h"

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
      (String("FarFramework\\")+e.GetDialog()->Name()+"\\"+e.Name()).ToAnsi(
        e.HistoryId, sizeof(e.HistoryId));
      item.History=e.HistoryId;
    }
  }
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
      strncpy(e.list.Items[i].Text, items[i].optr(), sizeof(e.list.Items[i].Text));
      if (obj("Text")==items[i])
        e.list.Items[i].Flags|=LIF_SELECTED;
    }
  }
}

int lablen(FarDialogItem& item)
{
  if (item.Flags & DIF_SHOWAMPERSAND)
    return (int)strlen(item.Data);
  else
  {
    int res=0;
    for (char *p=item.Data; *p; p++)
      if (*p!='&') res++;
    return res;
  }
}

