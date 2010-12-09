#pragma once
#include "object.h"
#include "valuelist.h"
#include "far/interface/plugin.hpp"

class FarDlgObject;
class FarDialog;

class FarDlgObjectClass : public ObjectClass
{
protected:
  void DefineProperties();

public:
  virtual void InitItem(FarDialogItem&, FarDlgObject&) { ; }
  virtual void RetrieveProperties(Array<FarDialogItem>& items, FarDlgObject&, HANDLE) { ; }
  virtual void BeforeAdd(FarDialogItem&, FarDlgObject&) { ; }
  virtual void LoadState(PropertyList &state, FarDlgObject&) { ; }
  virtual void SaveState(PropertyList &state, FarDlgObject&) { ; }
};

static struct { char* Name; int Flag; } Attrib[] = {
//  { "SetColor", DIF_SETCOLOR },
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
//  { "VarEdit", DIF_VAREDIT }, 
  { "Hidden", DIF_HIDDEN }, 
  { "ManualAddHistory", DIF_MANUALADDHISTORY }, 
};

class FarDlgLineClass : public FarDlgObjectClass
{
  DEFINE_CLASS("FarDlgLine", FarDlgObject)
  void DefineProperties()
  {
    FarDlgObjectClass::DefineProperties();
  }
  void InitItem(FarDialogItem& item, FarDlgObject& obj)
  {
    item.Type=DI_TEXT;
    item.Flags|=DIF_SEPARATOR | DIF_BOXCOLOR;
    item.PtrData=L"";
  }
};

int lablen(FarDialogItem& item);
String FormatWidth(const String& v, int len);

class FarDlgLabelClass : public FarDlgObjectClass
{
  DEFINE_CLASS("FarDlgLabel", FarDlgObject)
  void DefineProperties()
  {
    FarDlgObjectClass::DefineProperties();
    AddProperty("Shorten", 0);
  }
  void InitItem(FarDialogItem& item, FarDlgObject& obj)
  {
    item.Type=DI_TEXT;
    if(!obj("Shorten"))
		item.X2=item.X1+lablen(item)-1;
    else item.X2=item.X1-1;
  }
  void BeforeAdd(FarDialogItem& item, FarDlgObject& obj)
  {
//     if (obj("Shorten"))
//       FormatWidth(item.PtrData, __min(item.X2-item.X1+1, 500))
//       .ToAnsi(item.PtrData, sizeof(item.PtrData));
  }
};

class FarDlgButtonClass : public FarDlgObjectClass
{
  DEFINE_CLASS("FarDlgButton", FarDlgObject)
  void DefineProperties()
  {
    FarDlgObjectClass::DefineProperties();
    AddProperty("Default", 0);
    AddProperty("Result", -1);
  }
  virtual void InitItem(FarDialogItem& item, FarDlgObject& obj)
  {
    item.Type=DI_BUTTON;
    item.X2=item.X1+lablen(item)+4-1;
    if (obj("Default")) item.DefaultButton=1;
  }
};

class FarDlgPanelClass : public FarDlgObjectClass
{
  DEFINE_CLASS("FarDlgPanel", FarDlgContainer)
  void DefineProperties()
  {
    FarDlgObjectClass::DefineProperties();
  }
};

class FarDlgCheckboxClass : public FarDlgObjectClass
{
  DEFINE_CLASS("FarDlgCheckbox", FarDlgObject)
  void DefineProperties()
  {
    FarDlgObjectClass::DefineProperties();
    AddProperty("Selected", 0);
  }
  void InitItem(FarDialogItem& item, FarDlgObject& obj)
  {
    item.Type=DI_CHECKBOX;
    item.X2=item.X1+lablen(item)+4-1;
    item.Selected=obj("Selected");
  }
  void RetrieveProperties(Array<FarDialogItem>& items, FarDlgObject& obj, HANDLE dlg)
  {
    obj("Selected")=items[obj.DialogItem].Selected;
  }
  void LoadState(PropertyList &state, FarDlgObject& obj)
  {
    obj("Selected")=state[obj.Name()];
  }
  void SaveState(PropertyList &state, FarDlgObject& obj)
  {
    state[obj.Name()]=obj("Selected");
  }
};

class FarDlgRadioButtonClass : public FarDlgCheckboxClass
{
  DEFINE_CLASS("FarDlgRadioButton", FarDlgObject)
  void DefineProperties()
  {
    FarDlgCheckboxClass::DefineProperties();
  }
  void InitItem(FarDialogItem& item, FarDlgObject& obj)
  {
    FarDlgCheckboxClass::InitItem(item, obj);
    item.Type=DI_RADIOBUTTON;
  }
};

class FarDlgEditClass : public FarDlgObjectClass
{
  DEFINE_CLASS("FarDlgEdit", FarDlgEdit)
  void DefineProperties()
  {
    FarDlgObjectClass::DefineProperties();
    AddProperty("HistoryId", "FarFramework\\DefaultEditHistory");
    AddProperty("Width", 10);
  }
  void InitItem(FarDialogItem& item, FarDlgObject& obj);
  void RetrieveProperties(Array<FarDialogItem>& items, FarDlgObject& obj, HANDLE dlg);
  void LoadState(PropertyList &state, FarDlgObject& obj)
  {
    obj("Text")=state[obj.Name()];
  }
  void SaveState(PropertyList &state, FarDlgObject& obj)
  {
    state[obj.Name()]=obj("Text");
  }
  void BeforeAdd(FarDialogItem& item, FarDlgObject& obj)
  {
    if (obj("History"))
      item.X2--;
  }
};

class FarDlgComboboxClass : public FarDlgObjectClass
{
  DEFINE_CLASS("FarDlgCombobox", FarDlgCombobox)
  void DefineProperties()
  {
    FarDlgObjectClass::DefineProperties();
    AddProperty("Width", 10);
    AddProperty("Items", "");
  }
  void InitItem(FarDialogItem& item, FarDlgObject& obj);
  void RetrieveProperties(Array<FarDialogItem>& items, FarDlgObject& obj, HANDLE dlg)
  {
//    _toansi(items[obj.DialogItem].PtrData);
    obj("Text")=items[obj.DialogItem].PtrData;
  }
  void LoadState(PropertyList &state, FarDlgObject& obj)
  {
    obj("Text")=state[obj.Name()];
  }
  void SaveState(PropertyList &state, FarDlgObject& obj)
  {
    state[obj.Name()]=obj("Text");
  }
  void BeforeAdd(FarDialogItem& item, FarDlgObject& obj)
  {
    item.X2--;
  }
};

class FarDialogClass : public FarDlgObjectClass
{
  DEFINE_CLASS("FarDialog", FarDialog)
  void DefineProperties()
  {
    FarDlgObjectClass::DefineProperties();
    AddProperty("HelpTopic", "NoTopic");
    AddProperty("Title", "");
    AddProperty("Warning", 0);
  }
};


class FarDlgObjectReg
{
public:
  FarDlgObjectReg()
  {
    objectManager->RegisterClass(new FarDlgLineClass);
    objectManager->RegisterClass(new FarDlgLabelClass);
    objectManager->RegisterClass(new FarDlgPanelClass);
    objectManager->RegisterClass(new FarDlgButtonClass);
    objectManager->RegisterClass(new FarDlgCheckboxClass);
    objectManager->RegisterClass(new FarDlgRadioButtonClass);
    objectManager->RegisterClass(new FarDlgEditClass);
    objectManager->RegisterClass(new FarDlgComboboxClass);
    objectManager->RegisterClass(new FarDialogClass);
  }
};

extern FarDlgObjectReg* __FarDlgObjectReg;
