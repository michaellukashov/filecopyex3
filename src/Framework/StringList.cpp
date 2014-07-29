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

#include "StdHdr.h"
#include "LowLevelStr.h"
#include "StringList.h"

void StringList::Set(size_t n, const String & v)
{
  ListItem item;
  item.str = v;
  item.Data = 0;
  items[n] = item;
}

int & StringList::Values(size_t n)
{
  return items[n].Data;
}

void StringList::Add(const String & v, int data)
{
  ListItem itm;
  itm.Data = data;
  itm.str = v;
  items.push_back(itm);
}

/*
void StringList::Delete(int n)
{
  items.Delete(n, 1);
}

void StringList::Exchange(int i, int j)
{
  items.Exchange(i, j);
}
*/

void StringList::AddList(StringList & src)
{
  for (size_t i = 0; i < src.Count(); i++)
  {
    Add(src[i]);
  }
}

intptr_t StringList::Find(const String & v, intptr_t start) const
{
  for (size_t i = start; i < Count(); ++i)
  {
    if (items[i].str.cmp(v) == 0)
      return i;
  }
  return -1;
}
