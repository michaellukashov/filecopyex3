/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010
Idea & core: Max Antipin
Coding: Serge Cheperis aka craZZy
Bugfixes: slst, CDK, Ivanych, Alter, Axxie and Nsky
Special thanks to Vitaliy Tsubin
Far 2 (32 & 64 bit) full unicode version by djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __STRINGLIST_H__
#define __STRINGLIST_H__

#pragma once

#include <vector>

#include "objString.h"
#include "StringParent.h"

#define slSorted 1
#define slIgnoreCase 2

class StringList: public StringParent
{
public:
  virtual void Clear() { items.clear(); };
  virtual const String & operator[](size_t i) const { return items[i].str; };
  virtual void AddString(const String & v) { Add(v); };
  virtual size_t Count() const { return items.size(); };

  int & Values(int);
  void Set(int, const String &);
  void Add(const String &, int=0);
  //void Delete(int);
  //void Exchange(int, int);

  int Find(const String &, int=0) const;

  void AddList(StringList &);

  struct ListItem
  {
    String str;
    int Data;
  };

private:
  std::vector<ListItem> items;
};

#endif//__STRINGLIST_H__
