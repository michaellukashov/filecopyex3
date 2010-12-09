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
  Handle Str;
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
