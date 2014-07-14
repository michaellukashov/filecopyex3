#include "FarMenu.h"

#include "common.h"
#include "guid.hpp"

FarMenu::FarMenu(): Flags(0), Selection(0)
{
}

FarMenu::~FarMenu()
{
  for (size_t i = 0; i < items.size(); ++i)
  {
    delete[] items[i].Text;
  }
}

void FarMenu::SetItemText(FarMenuItem * item, const String & text)
{
  size_t len = text.len() + 1;
  wchar_t * t = new wchar_t[len];
  text.copyTo(t, len);
  item->Text = t;
}

void FarMenu::AddLine(const String & line)
{
  FarMenuItem item;
  item.Flags = 0;
  if (Selection == items.size())
  {
    item.Flags = MIF_SELECTED;
  }
  SetItemText(&item, line);
  items.push_back(item);
}

void FarMenu::AddLineCheck(const String & line, int check)
{
  FarMenuItem item;
  item.Flags = 0;
  if (check)
  {
    item.Flags |= MIF_CHECKED;
  }
  if (Selection == items.size())
  {
    item.Flags = MIF_SELECTED;
  };
  SetItemText(&item, line);
  items.push_back(item);
}

void FarMenu::AddSep()
{
  FarMenuItem item;
  item.Flags = MIF_SEPARATOR;
  SetItemText(&item, String());
  items.push_back(item);
}


intptr_t FarMenu::Execute()
{
  return Info.Menu(&MainGuid, &MenuGuid, -1, -1, 0, FMENU_WRAPMODE, Title.c_str(), NULL, HelpTopic.c_str(), NULL, NULL, items.data(), items.size());
}

void FarMenu::SetBottom(const String & v)
{
  Bottom=v;
}

void FarMenu::SetFlags(int f)
{
  Flags=f;
}

void FarMenu::SetHelpTopic(const String & v)
{
  HelpTopic=v;
}

void FarMenu::SetSelection(int n)
{
  Selection=n;
}

void FarMenu::SetTitle(const String & v)
{
  Title=v;
}
