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

#include "stringlist.h"
#include "array.h"

enum ValType { vtInt, vtFloat, vtString };

class PropVal
{
public:
  ValType Type;
  int Int;
  double Float;
  String Str;
};

class Property : private PropVal
{
public:
  void operator= (int);
  void operator= (double);
  void operator= (const String&);
  void operator= (const Property&);
  operator int() const;
  operator bool() const;
  operator double() const;
  operator const String() const;
  bool operator== (const String&) const;
  bool operator== (int) const;
  bool operator== (double) const;
  bool operator== (const Property&) const;
  bool operator!= (const String&) const;
  bool operator!= (int) const;
  bool operator!= (double) const;
  bool operator!= (const Property&) const;
  bool operator!() const;

  friend class PropertyStore;
};

extern Property UndefProperty;

class PropertyStore
{
public:
  PropertyStore(void);
  virtual ~PropertyStore(void);

  Property& operator[](int);

  int Add(int);
  int Add(double);
  int Add(const String&);

  int Count();
  void CopyFrom(PropertyStore&);

  void SaveToList(StringList&, StringList&);
  void LoadFromList(StringList&, StringList&);

private:
  Array<PropVal> Values;
  void Clear();

  friend class Property;
};

class PropertyList : protected PropertyStore
{
public:
  PropertyList(void) { ; }
  virtual ~PropertyList(void) { ; }

  Property& operator[](int i) { return store[i]; }
  Property& operator[](const String& n)
  {
    int i=names.Find(n);
    if (i!=-1) return store[i];
    else return UndefProperty;
  }

  void GetNameAndValue(int i, String& name, String& value)
  {
    name  = (String) names[i];
    value = (String) store[i];
  }

  Property& GetValueByName(const String& n)
  {
    int i=names.Find(n);
    if (i!=-1) return store[i];
    else return UndefProperty;
  }

  int Add(const String& n, int v) { return names.Add(n, store.Add(v)); }
  int Add(const String& n, double v) { return names.Add(n, store.Add(v)); }
  int Add(const String& n, const String& v) { return names.Add(n, store.Add(v)); }

  int Count() { return names.Count(); }

  void SaveToList(StringList& l) { store.SaveToList(l, names); }
  void LoadFromList(StringList& l) { store.LoadFromList(l, names); }

private:
  PropertyStore store;
  StringList names;
};
