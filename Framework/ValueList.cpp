#include <stdhdr.h>
#include "lowlevelstr.h"
#include "valuelist.h"

// int Compare(const ValueList::ListItem& s1, const ValueList::ListItem& s2,
//             const void* p)
// {
//   int res=_tcsicmp(
//     heap->LockString(s1.name), 
//     heap->LockString(s2.name));
//   heap->Unlock(s1.name);
//   heap->Unlock(s2.name);
//   return res;
// }

ValueList::ValueList()
{
//   InitHeap();
//   items.SetSorted(1, Compare);
}

ValueList::~ValueList()
{
  Clear();
}

const String ValueList::Name(int n)
{
  return items[n].name;
}

const String ValueList::Value(int n)
{
  return items[n].value;
}

int ValueList::Count()
{
  return items.Count();
}

void ValueList::Set(const String& n, const String& v)
{
  int i = Find(n);
  if(i != -1)
  {
    items[i].value = v;
  }
  else 
  {
	  ListItem itm;
    itm.name=n;
    itm.value=v;
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

//=============================================================================
//	ValueList::Find
//-----------------------------------------------------------------------------
int ValueList::Find(const String& k)
{
	for(int i = 0; i < items.Count(); ++i)
	{
		if(items[i].name.icmp(k) == 0)
			return i;
	}
	return -1;
}

const String ValueList::operator[](const String& n)
{
  int i = Find(n);
  if (i != -1)
	  return Value(i);
  else
	  return String();
}

void ValueList::Clear()
{
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