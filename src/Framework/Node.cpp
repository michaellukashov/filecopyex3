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

#include "stdhdr.h"
#include "Node.h"
#include "../common.h"
#include "lowlevelstr.h"
#include "strutils.h"
#include "ObjectManager.h"

Property& Node::operator()(const String& v) 
{ 
	return getPayload()(v); 
}

const String Node::getName() const 
{ 
	return getPayload().getName(); 
}

/*
const String Node::getType() const 
{ 
	return getPayload().getType(); 
}
*/

int Node::LoadFrom(FILE *f)
{
	StringVector temp;
	return temp.loadFromFile(f) && LoadFromList(temp);
}

/*
int Node::SaveTo(FILE *f)
{
	StringVector temp;
	SaveToList(temp);
	return temp.saveToFile(f);
}
*/

int Node::Load(const String& fn)
{
	StringVector temp;
	return temp.loadFromFile(fn) && LoadFromList(temp);
}

/*
int Node::Save(const String& fn)
{
	StringVector temp;
	SaveToList(temp);
	return temp.saveToFile(fn);
}
*/

Node& Node::child(const String& v)
{
	for(size_t i = 0; i < childs.size(); ++i)
	{
		if(childs[i]->getName() == v) {
			return child(i);
		}
	}
	FWError(Format(L"Request to undefined object %s", v.ptr()));
	return Node();
}

void Node::ClearChilds()
{
	for(size_t i = 0; i < childs.size(); ++i)
	{
		delete childs[i];
	}
	childs.clear();
}

size_t Node::LoadFromList(StringParent &list, size_t start)
{
	ClearChilds();
	BeforeLoad();

	size_t res = list.Count() - 1;

	for (size_t i = start; i < list.Count(); i++) {
		String line = list[i].trim();
		if (line == "end") {
			res=i;
			break;
		} else if (!line.ncmp("object", 6)) {
			int p = line.find(' ');
			int p1 = line.find(':');
			if (p!=-1 && p1!=-1 && p<p1) {
				String pname = line.substr(p+1, p1-p-1).trim();
				String ptype = line.substr(p1+1).trim();
				Node *obj = objectManager->create(ptype, pname, this);
				if (obj) {
					i = obj->LoadFromList(list, i+1);
				} else {
					FWError(Format(L"Object type %s is undefined", ptype.ptr()));
				}
			}
		} else  {
			int p=line.find('=');
			if (p!=-1)	{
				String pline=line.substr(0, p).trim();
				String pval=line.substr(p+1).trim().trimquotes();
				(*this)(pline)=pval;
			}
		}
	}

	AfterLoad();
	getPayload().propSave(); // loadedProp = prop;
	return res;
}

/*
void Node::SaveToList(StringVector &list, int clear, int level)
{
	if (clear) list.Clear();
	String pfx;
	for (int j=0; j<level; j++) pfx+="  ";
	for (int i=0; i<_class->PropertyNames.Count(); i++)
	{
		list.Add(pfx+_class->PropertyNames[i]+"="
			+properties[_class->PropertyNames.Values(i)]);
	}
	for (int i=0; i<childs.Count(); i++)
	{
		list.Add(pfx+"object "+childs[i]->Name()+": "+childs[i]->Type());
		childs[i]->SaveToList(list, 0, level+1);
		list.Add(pfx+"end");
	}
}
*/

void Node::ReloadProperties()
{
	getPayload().propLoad();
}

void Node::ReloadPropertiesRecursive()
{
	ReloadProperties();
	for (int i=0; i < childs.size(); i++) {
		childs[i]->ReloadPropertiesRecursive();
	}
}

/*
void Node::init(const String &name, const String &type, ObjectClass* cl, Object* parent) {
	_class = cl;
	//!!! _type = type;
	//!!! _name = name;
	_parent = parent;
	//!!! prop = cl->getProps();
	if (parent) {
		parent->childs.Add(this);
	}
};
*/