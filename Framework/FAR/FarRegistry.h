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

#include "../stringlist.h"

class FarRegistry
{
public:
  FarRegistry(void);
  virtual ~FarRegistry(void);
  int GetInt(const String&, const String&, int);
  void SetInt(const String&, const String&, int);
  String GetString(const String&, const String&, const String&);
  void SetString(const String&, const String&, const String&);
  void ReadList(const String&, StringList&);
  void WriteList(const String&, StringList&);
  void DeleteKey(const String&);
  void CopyKey(const String&, const String&);
protected:
  String ResolveKey(const String&);
  HKEY OpenKey(const String&);
  HKEY CreateKey(const String&);
};

extern FarRegistry Registry;
