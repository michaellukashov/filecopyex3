#pragma once

#include <vector>

#include "Framework/ObjString.h"
#include "SDK/plugin.hpp"

class FarMenu
{
public:
  FarMenu();
  virtual ~FarMenu();

  void SetTitle(const String &);
  void SetBottom(const String &);
  void SetHelpTopic(const String &);
  void SetFlags(DWORD f);
  void AddLine(const String &);
  void AddLineCheck(const String &, int check);
  void AddSep();
  void SetSelection(size_t n);
  intptr_t Execute();

protected:
  String Title, Bottom, HelpTopic;
  DWORD Flags;
  size_t Selection;
  std::vector<FarMenuItem> items;

  static void SetItemText(FarMenuItem * item, const String & text);
};

