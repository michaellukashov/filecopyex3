/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010 Serge Cheperis aka craZZy
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

#include "stdhdr.h"
#include "object.h"
#include "fwcommon.h"
#include "lowlevelstr.h"
#include "strutils.h"

ObjectManager *objectManager;

void ObjectClass::AddProperty(const String& name, int def)
{
  PropertyNames.Add(name, Properties.Add(def));
}

void ObjectClass::AddProperty(const String& name, double def)
{
  PropertyNames.Add(name, Properties.Add(def));
}

void ObjectClass::AddProperty(const String& name, const String& def)
{
  PropertyNames.Add(name, Properties.Add(def));
}

int Object::LoadFrom(FILE *f)
{
  StringList temp;
  return temp.LoadFrom(f) && LoadFromList(temp);
}

int Object::SaveTo(FILE *f)
{
  StringList temp;
  SaveToList(temp);
  return temp.SaveTo(f);
}

Property& Object::Property(const String& v)
{
  int j=_class->PropertyNames.Find(v);
  if (j!=-1)
    return Properties[_class->PropertyNames.Values(j)];
  else 
    return UndefProperty;
}

int Object::Load(const String& fn)
{
  StringList temp;
  return temp.Load(fn) && LoadFromList(temp);
}

int Object::Save(const String& fn)
{
  StringList temp;
  SaveToList(temp);
  return temp.Save(fn);
}

int Object::LoadFromList(StringList &list, int start)
{
  Children.Clear();
  BeforeLoad();

  int i=start, res=list.Count()-1;
  while (i<list.Count())
  {
    String line=list[i].trim();
    if (line=="end")
    {
      res=i;
      break;
    }
    else if (!line.ncmp("object", 6))
    {
      int p=line.cfind(' '), p1=line.cfind(':');
      if (p!=-1 && p1!=-1 && p<p1)
      {
        String pname=line.substr(p+1, p1-p-1).trim();
        String ptype=line.substr(p1+1).trim();
        Object *obj=objectManager->Create(ptype, pname, this);
        if (obj)
          i=obj->LoadFromList(list, i+1);
        else
          FWError(Format("Object type %s is undefined", ptype));
      }
    }
    else 
    {
      int p=line.cfind('=');
      if (p!=-1)
      {
        String pline=line.substr(0, p).trim();
        String pval=line.substr(p+1).trim().trimquotes();
        (*this)(pline)=pval;
      }
    }
    i++;
  }

  AfterLoad();
  LoadedProperties.CopyFrom(Properties);
  return res;
}

void Object::SaveToList(StringList &list, int clear, int level)
{
  if (clear) list.Clear();
  String pfx;
  for (int j=0; j<level; j++) pfx+="  ";
  for (int i=0; i<_class->PropertyNames.Count(); i++)
  {
    list.Add(pfx+_class->PropertyNames[i]+"="
      +Properties[_class->PropertyNames.Values(i)]);
  }
  for (int i=0; i<Children.Count(); i++)
  {
    list.Add(pfx+"object "+Children[i].Name()+": "+Children[i].Type());
    Children[i].SaveToList(list, 0, level+1);
    list.Add(pfx+"end");
  }
}

void Object::ReloadProperties()
{
  Properties.CopyFrom(LoadedProperties);
}

void Object::ReloadPropertiesRecursive()
{
  ReloadProperties();
  for (int i=0; i<Children.Count(); i++)
    Children[i].ReloadPropertiesRecursive();
}

ObjectList::ObjectList()
{ 
  Parent=NULL; 
}

ObjectList::~ObjectList()
{
  Clear();
}
 
void ObjectList::Add(Object& obj)
{
  StringList::Add(obj._name, &obj);
}

void ObjectList::Delete(int n)
{
  delete &((*this)[n]);
  StringList::Delete(n);
}

void ObjectList::Clear()
{
  for (int i=0; i<Count(); i++)
    delete &((*this)[i]);
  StringList::Clear();
}

int ObjectList::Count()
{
  return StringList::Count();
}

Object& ObjectList::operator[] (int n)
{
  return *((Object*)StringList::PtrValues(n));
}

Object& ObjectList::operator[] (const String& n)
{
  int i=StringList::Find(n);
  if (i!=-1) return operator[](i);
  FWError(Format("Request to undefined object %s", n));
  return *((Object*)NULL);
}

void InitObjMgr()
{
  objectManager=new ObjectManager();
}
