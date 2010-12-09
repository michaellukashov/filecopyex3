#include <stdhdr.h>
#include "lowlevelstr.h"
#include "valuelist.h"

int Compare(const ValueList::ListItem& s1, const ValueList::ListItem& s2,
            const void* p)
{
  int res=_tcsicmp(
    heap->LockString(s1.name), 
    heap->LockString(s2.name));
  heap->Unlock(s1.name);
  heap->Unlock(s2.name);
  return res;
}

ValueList::ValueList()
{
  InitHeap();
  items.SetSorted(1, Compare);
}

ValueList::~ValueList()
{
  Clear();
}

const String ValueList::Name(int n)
{
  return String(items[n].name);
}

const String ValueList::Value(int n)
{
  return String(items[n].value);
}

int ValueList::Count()
{
  return items.Count();
}

void ValueList::Set(const String& n, const String& v)
{
  ListItem itm;
  itm.name=n.handle();
  int i=items.Find(itm);
  if (i!=-1) 
  {
    heap->ReallocString(items[i].value, v);
  }
  else 
  {
    itm.name=heap->AllocString(n);
    itm.value=heap->AllocString(v);
    items.Add(itm);
  }
}

void ValueList::Set(const String& s)
{
  int p=s.find("=");
  if (p!=-1)
    Set(s.substr(0, p).trim().trimquotes(), 
      s.substr(p+1).trim().trimquotes());
}

const String ValueList::operator[](const String& n)
{
  ListItem itm;
  itm.name=n.handle();
  int i=items.Find(itm);
  if (i!=-1) return Value(i);
  else return String();
}

void ValueList::Clear()
{
  for (int i=0; i<Count(); i++)
  {
    heap->Free(items[i].name);
    heap->Free(items[i].value);
  }
  items.Clear();
}

void ValueList::LoadFromList(StringList& list)
{
  Clear();
  for (int i=0; i<list.Count(); i++)
    Set(list[i]);
}

void ValueList::SaveToList(StringList& list)
{
  list.Clear();
  for (int i=0; i<Count(); i++)
    list.Add(Name(i)+"="+Value(i));
}

void LocaleList::Load(const String& fn)
{
  StringList temp;
  if (temp.Load(fn)) LoadFromList(temp);
}