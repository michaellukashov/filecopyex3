#pragma once
#include "array.h"
#include "object.h"
#include "far/interface/plugin.hpp"

class FarDlgObjectClass;
class FarDialogClass;

struct RetCode
{
  int itemNo, retCode;
};

class FarDialog;
class ValueList;

class FarDlgObject : public CastObject 
  <FarDlgObject, FarDlgObject, FarDlgObjectClass>
{
public:
  FarDlgObject(void);
  virtual ~FarDlgObject(void);

  virtual void InitItem(FarDialogItem& item);
  virtual void RetrieveProperties(Array<FarDialogItem>& items);
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
  char HistoryId[64];
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
  void RetrieveProperties(Array<FarDialogItem>& items);

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

  void LoadState(PropertyList &state)
    { FarDlgContainer::LoadState(state); }
  void SaveState(PropertyList &state)
    { FarDlgContainer::SaveState(state); }

protected:
  void BeforeLoad();
};

class FarDialogList : public 
  CastObject<FarDialog, FarDialogList, FarDialogClass>
{
};

