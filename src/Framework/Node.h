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

#include "Properties.h"
#include "StringVector.h"

class Payload;

class Node
{
public:
  Node();
  virtual ~Node();

  virtual void init(Payload * _payload, Node * _parent);

  Node & operator[](size_t i) { return child(i); }
  Node & operator[](const String & v) { return child(v); }
  Property & operator()(const String & name);

  Payload & getPayload() const;
  Node * getParent() { return parent; }
  const String getName() const;
  //const String getType() const;

  int LoadFrom(FILE *);
  bool Load(const String &);
  void ReloadProperties() const;
  void ReloadPropertiesRecursive();

  //void init(const String &name, const String &type, ObjectClass* cl, Object* parent);

protected:
  void ClearChilds();
  Node & child(size_t i) { return *childs[i]; }
  Node & child(const String & v);

  virtual void AfterLoad() {}
  virtual void BeforeLoad() {}

  std::vector<Node *> childs;
  Payload * payload;
  Node * parent;

private:
  size_t LoadFromList(StringParent &, size_t start = 0);
  //void SaveToList(StringVector&, int clear=1, int level=0);

private:
  Node(const Node &);
  Node & operator = (const Node &);
};

#define DEFINE_NODE_CLASS(type) \
  static Node* create() { return new type(); }
