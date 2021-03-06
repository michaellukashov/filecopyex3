#include "FarMenu.h"

#include "Common.h"
#include "guid.hpp"

FarMenu::FarMenu() :
  Flags(0), Selection(0)
{
}

FarMenu::~FarMenu()
{
  for (size_t Index = 0; Index < items.size(); ++Index)
  {
    delete[] items[Index].Text;
  }
}

void FarMenu::SetItemText(FarMenuItem * item, const String & text)
{
  size_t len = text.len() + 1;
  delete[] item->Text;
  wchar_t * t = new wchar_t[len];
  text.copyTo(t, len);
  item->Text = t;
}

void FarMenu::AddLine(const String & line)
{
  FarMenuItem item;
  ::ZeroMemory(&item, sizeof(item));
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
  ::ZeroMemory(&item, sizeof(item));
  item.Flags = 0;
  if (check)
  {
    item.Flags |= MIF_CHECKED;
  }
  if (Selection == items.size())
  {
    item.Flags = MIF_SELECTED;
  }
  SetItemText(&item, line);
  items.push_back(item);
}

void FarMenu::AddSep()
{
  FarMenuItem item;
  ::ZeroMemory(&item, sizeof(item));
  item.Flags = MIF_SEPARATOR;
  SetItemText(&item, String());
  items.push_back(item);
}

intptr_t FarMenu::Execute()
{
  return Info.Menu(&MainGuid, &MenuGuid, -1, -1, 0, FMENU_WRAPMODE, Title.c_str(), nullptr, HelpTopic.c_str(), nullptr, nullptr, items.data(), items.size());
}

void FarMenu::SetBottom(const String & v)
{
  Bottom = v;
}

void FarMenu::SetFlags(DWORD f)
{
  Flags = f;
}

void FarMenu::SetHelpTopic(const String & v)
{
  HelpTopic = v;
}

void FarMenu::SetSelection(size_t n)
{
  Selection = n;
  if (Selection < items.size())
  {
    for (size_t I = 0; I < items.size(); I++)
    {
      FarMenuItem & item = items[I];
      if (I != Selection)
        item.Flags &= ~MIF_SELECTED;
      else
        item.Flags |= MIF_SELECTED;
    }
  }
}

void FarMenu::SetTitle(const String & v)
{
  Title = v;
}
