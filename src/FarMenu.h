#ifndef __FARMENU_H__
#define __FARMENU_H__

#pragma once

#include <vector>

#include "Framework/ObjString.h"
#include "SDK/plugin.hpp"

class FarMenu
{
public:
	FarMenu();
	virtual ~FarMenu();
	void SetTitle(const String&);
	void SetBottom(const String&);
	void SetHelpTopic(const String&);
	void SetFlags(int f);
	void AddLine(const String&);
	void AddLineCheck(const String&, int check);
	void AddSep();
	void SetSelection(int n);
	int Execute();
protected:
	void SetItemText(FarMenuItem* item, const String& text);
	String Title, Bottom, HelpTopic;
	int Flags, Selection;
	std::vector<FarMenuItem> items;
};

#endif