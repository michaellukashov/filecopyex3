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

#pragma once

#include <vector>

#include "ObjString.h"

class FileName
{
public:
  enum Direction
  {
    levelPlus,
    levelMinus,
    levelSame,
    levelStar,
  };

  FileName(const Direction _d, const String & _name): d(_d), Name(_name) {}
  ~FileName() {}

  Direction getDirection() const { return d; }
  String getName() const { return Name; }

private:
  Direction d;
  String Name;
};

class FileNameStore
{
public:
  FileNameStore() { }
  ~FileNameStore() { }

  size_t AddRel(FileName::Direction _d, const String & _name) { items.push_back(FileName(_d, _name)); return items.size() - 1; }
  size_t Count() const { return items.size(); }
  const String GetNameByNum(size_t n) const { return items[n].getName(); }
  const FileName & operator[](size_t n) const { return items[n]; }
  FileName & operator[](size_t n) { return items[n]; }

private:
  std::vector<FileName> items;
};
