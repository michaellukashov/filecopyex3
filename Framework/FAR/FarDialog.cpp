#include <stdhdr.h>
#include "fwcommon.h"
#include "object.h"
#include "array.h"
#include "strutils.h"
#include "far/farplugin.h"

extern PluginStartupInfo Info;

FarDialog::FarDialog(void)
{
}

FarDialog::~FarDialog(void)
{
}

void SetItemText(FarDialogItem* item, const String& text);

int FarDialog::Execute()
{
  Array<FarDialogItem> Items;
  Array<RetCode> RetCodes;
  FarDialogItem frame;
  memset(&frame, 0, sizeof(frame));
  frame.Type=DI_DOUBLEBOX;
  String p=Property("Title");
  if (p=="") p=LOC(Name());
  SetItemText(&frame, p);
  Items.Add(frame);

  int w, h, f;
  DefSize(w, h, f);
  AddToItems(Items, RetCodes, 5, 2, w);

  Items[0].X1=3; Items[0].Y1=1; 
  Items[0].X2=w+6; Items[0].Y2=h+2;

  HANDLE hnd=Info.DialogInit(Info.ModuleNumber, -1, -1, w+10, h+4, 
    String(Property("HelpTopic")).ptr(), 
    (FarDialogItem*)Items.Storage(), Items.Count(),
    0, bool(Property("Warning")) ? FDLG_WARNING : 0,
    Info.DefDlgProc, 0);
  int ret=-1;
  if (hnd!=INVALID_HANDLE_VALUE)
  {
	  int res = Info.DialogRun(hnd);
    for (int i=0; i<RetCodes.Count(); i++)
      if (RetCodes[i].itemNo==res)
      {
        if (RetCodes[i].retCode!=-1)
          RetrieveProperties(Items, hnd);
        ret=RetCodes[i].retCode;
        break;
      }
  }
  ClearDialogItem();
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
    FWError(Format("Request to undefined object %s", n));
  return *obj;
}

