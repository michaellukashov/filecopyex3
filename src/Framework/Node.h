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

#ifndef	__OBJECT_H__
#define	__OBJECT_H__

#pragma once

//#include "Array.h"
#include "Properties.h"
#include "Payload.h"
#include "StringVector.h"

class Payload;

class Node
{
public:
	//Object(ObjectClass* __class) { _class = __class; _parent = NULL; };
	Node(Payload* _payload, Node *_parent) { 
		payload = _payload; 
		parent = _parent; 
		if (parent) {
			parent->childs.Add(this);
		}
	};
	virtual ~Node() { ClearChilds(); }

	Node& operator[](size_t i) { return child(i); };
	Node& operator[](const String& v) { return child(v); };
	Property& operator()(const String& name);

	Payload& getPayload() const { return *payload; }
	Node* getParent() { return parent; }
	const String getName() const;
	//const String getType() const;

	int LoadFrom(FILE*);
	//int SaveTo(FILE*);
	int Load(const String&);
	//int Save(const String&);
	void ReloadProperties();
	void ReloadPropertiesRecursive();

	//void init(const String &name, const String &type, ObjectClass* cl, Object* parent);

protected:
	void ClearChilds();
	Node& child(size_t i) { return *childs[i]; }
	Node& child(const String& v);

	Node() {}
	virtual void AfterLoad() {}
	virtual void BeforeLoad() {}

	//PropertyMap prop;
	//PropertyMap loadedProp;

	Array<Node*> childs;
	Payload* payload;
	Node* parent;

private:
	size_t LoadFromList(StringParent&, size_t start = 0);
	//void SaveToList(StringVector&, int clear=1, int level=0);

};

#endif //__OBJECT_H__
