#include <stdhdr.h>
#include "lowlevelstr.h"
#include "properties.h"
#include "valuelist.h"
#include "fwcommon.h"

PropertyStore::PropertyStore(void)
{
  InitHeap();
}

PropertyStore::~PropertyStore(void)
{
  Clear();
}

Property& PropertyStore::operator [](int n)
{
  return static_cast<Property&>(Values[n]);
}

Property UndefProperty;

Property::operator int() const
{
  switch (Type)
  {
    case vtInt: return Int;
    case vtFloat: return (int)Float;
    case vtString: return String(Str).AsInt();
  }
  return 0;
}

Property::operator bool() const
{
  switch (Type)
  {
    case vtInt: return Int!=0;
    case vtFloat: return Float!=0;
    case vtString: return String(Str)!="";
  }
  return 0;
}

bool Property::operator!() const
{
  return !operator bool();
}

Property::operator double() const
{
  switch (Type)
  {
    case vtInt: return (double)Int;
    case vtFloat: return Float;
    case vtString: return String(Str).AsFloat();
  }
  return 0;
}

Property::operator const String() const
{
  switch (Type)
  {
    case vtInt: return String(Int);
    case vtFloat: return String(Float);
    case vtString: return String(Str);
  }
  return "";
}

void Property::operator=(int v)
{
  switch (Type)
  {
    case vtInt: Int=v; break;
    case vtFloat: Float=(double)v; break;
    case vtString: heap->ReallocString(Str, String(v)); 
  }
}

void Property::operator=(double v)
{
  switch (Type)
  {
    case vtInt: Int=(int)v; break;
    case vtFloat: Float=v; break;
    case vtString: heap->ReallocString(Str, String(v)); 
  }
}

void Property::operator=(const String& v)
{
  switch (Type)
  {
    case vtInt: Int=v.AsInt(); break;
    case vtFloat: Float=v.AsFloat(); break;
    case vtString: heap->ReallocString(Str, v);
  }
}

void Property::operator=(const Property& v)
{
  switch (Type)
  {
    case vtInt: Int=(int)v; break;
    case vtFloat: Float=(double)v; break;
    case vtString: heap->ReallocString(Str, String(v));
  }
}

bool Property::operator== (const String& v) const
{
  return v==operator const String();
}

bool Property::operator== (int v) const
{
  return v==operator int();
}

bool Property::operator== (double v) const
{
  return v==operator double();
}

bool Property::operator== (const Property& v) const
{
  switch (Type)
  {
    case vtInt: return operator==((int)v);
    case vtFloat: return operator==((double)v);
    case vtString: return operator==((const String)v);
  }
  return false;
}

bool Property::operator!= (const String& v) const
{
  return v!=operator const String();
}

bool Property::operator!= (int v) const
{
  return v!=operator int();
}

bool Property::operator!= (double v) const
{
  return v!=operator double();
}

bool Property::operator!= (const Property& v) const
{
  return !operator==(v);
}

int PropertyStore::Add(int v)
{
  PropVal val;
  val.Type=vtInt;
  val.Int=v;
  return Values.Add(val);
}

int PropertyStore::Add(double v)
{
  PropVal val;
  val.Type=vtFloat;
  val.Float=v;
  return Values.Add(val);
}

int PropertyStore::Add(const String& v)
{
  PropVal val;
  val.Type=vtString;
  val.Str=heap->AllocString(v);
  return Values.Add(val);
}

int PropertyStore::Count()
{
  return Values.Count();
}

void PropertyStore::Clear()
{
  for (int i=0; i<Count(); i++)
  {
    if (Values[i].Type==vtString)
      heap->Free(Values[i].Str);
  }
  Values.Clear();
}

void PropertyStore::SaveToList(StringList& list, StringList& names)
{
  ValueList temp;
  for (int i=0; i<Count(); i++)
    temp.Set(names[i], (*this)[names.Values(i)]);
  temp.SaveToList(list);
}

void PropertyStore::LoadFromList(StringList& list, StringList& names)
{
  ValueList temp;
  temp.LoadFromList(list);
  for (int i=0; i<temp.Count(); i++)
  {
    int j=names.Find(temp.Name(i));
    if (j!=-1)
      (*this)[names.Values(j)]=temp.Value(i);
  }
}

void PropertyStore::CopyFrom(PropertyStore& src)
{
  Clear();
  Values.Resize(src.Count());
  for (int i=0; i<src.Count(); i++)
  {
    PropVal val=src.Values[i];
    if (val.Type==vtString)
      val.Str=heap->AllocString(String(val.Str));
    Values[i]=val;
  }
}
