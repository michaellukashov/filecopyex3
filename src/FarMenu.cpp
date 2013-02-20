#include "FarMenu.h"

#include "common.h"
#include "guid.hpp"

FarMenu::FarMenu()
{
  Flags=Selection=0;
}

FarMenu::~FarMenu()
{
	for(int i = 0; i < items.size(); ++i)
	{
		free((void*)items[i].Text);
	}
}

void FarMenu::SetItemText(FarMenuItem* item, const String& text)
{
	size_t len = text.len() + 1;
	wchar_t* t = (wchar_t*)malloc(sizeof(wchar_t) * len);
	text.ToUnicode(t, len);
	item->Text = t;
}

void FarMenu::AddLine(const String& line)
{
  FarMenuItem item;
  item.Flags = 0;
  if (Selection == items.size()) {
	item.Flags = MIF_SELECTED;
  };
  SetItemText(&item, line);
  items.push_back(item);
}

void FarMenu::AddLineCheck(const String& line, int check)
{
	FarMenuItem item;
	item.Flags = 0;
	if (check) {
		item.Flags |= MIF_CHECKED;
	}
	if (Selection == items.size()) {
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

/*
intptr_t menu(const GUID& id, const wstring& title, const MenuItems& items, const wchar_t* help) {
  vector<FarMenuItem> menu_items;
  menu_items.reserve(items.size());
  FarMenuItem mi;
  for (unsigned i = 0; i < items.size(); i++) {
    memzero(mi);
    mi.Text = items[i].c_str();
    menu_items.push_back(mi);
  }
  return g_far.Menu(&c_plugin_guid, &id, -1, -1, 0, FMENU_WRAPMODE, title.c_str(), NULL, help, NULL, NULL, menu_items.data(), static_cast<int>(menu_items.size()));
}
*/

int FarMenu::Execute()
{
	//return Info.Menu(&MainGuid, &MenuGuid, -1, 0, Flags, Title.ptr(), Bottom.ptr(), HelpTopic.ptr(), NULL, NULL, items.Storage(), items.Count());
	return Info.Menu(&MainGuid, &MenuGuid, -1, -1, 0, FMENU_WRAPMODE, Title.c_str(), NULL, HelpTopic.c_str(), NULL, NULL, items.data(), items.size());
}

void FarMenu::SetBottom(const String& v)
{
  Bottom=v;
}

void FarMenu::SetFlags(int f)
{
  Flags=f;
}

void FarMenu::SetHelpTopic(const String& v)
{
  HelpTopic=v;
}

void FarMenu::SetSelection(int n)
{
  Selection=n;
}

void FarMenu::SetTitle(const String& v)
{
  Title=v;
}