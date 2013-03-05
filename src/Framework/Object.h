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
#include "ObjectClass.h"
#include "StringVector.h"

class ObjectClass;
class Object;

typedef Array<Object*> Objects;

class Object
{
public:
	virtual ~Object() { ClearChilds(); }

	Object& operator[](int i) { return Child(i); }
	Object& operator[](const String& v) { return Child(v); }
	Property& operator()(const String& v) { return Property(v); }

	ObjectClass* Class() { return _class; }
	Object* Parent() { return _parent; }
	const String& Name() const { return _name; }
	const String& Type() const { return _type; }

	int LoadFrom(FILE*);
	//int SaveTo(FILE*);
	int Load(const String&);
	//int Save(const String&);
	void ReloadProperties();
	void ReloadPropertiesRecursive();

	void init(const String &name, const String &type, ObjectClass* cl, Object* parent);

protected:
	void ClearChilds();
	Object& Child(int i) { return *childs[i]; }
	Object& Child(const String& v);
	Property& Property(const String& v);

	Object() {}
	virtual void AfterLoad() {}
	virtual void BeforeLoad() {}

	PropertyMap prop;
	PropertyMap loadedProp;

	Objects childs;
	ObjectClass* _class;
	Object* _parent;
	String _name;
	String _type;

private:
	size_t LoadFromList(StringParent&, size_t start = 0);
	//void SaveToList(StringVector&, int clear=1, int level=0);

};

#define DEFINE_CLASS(name, type) \
protected: \
	Object* Create() { return (Object*)new type; } \
	virtual const String TypeName() { return name; }


#endif //__OBJECT_H__
