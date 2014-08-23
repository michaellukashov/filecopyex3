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

#include "StdHdr.h"
#include "LowLevelStr.h"
#include "Properties.h"

Property::Property(const Property & other) :
  type(other.type),
  vInt(other.vInt),
  vFloat(other.vFloat),
  vStr(other.vStr)
{
}

Property::Property(int v) : vFloat(0.)
{
  type = vtInt;
  vInt = v;
}

Property::Property(float v) : vInt(0)
{
  type = vtFloat;
  vFloat = v;
}

Property::Property(const String & v) : type(vtString), vInt(0), vFloat(0.0), vStr(v)
{
}

Property & Property::operator=(const Property & p)
{
  type = p.type;
  switch (p.type)
  {
    case vtInt:
      vInt = (int)p;
      break;

    case vtFloat:
      vFloat = (float)p;
      break;

    case vtString:
      vStr = p.operator const String();
      break;
  }
  return *this;
}

Property::operator int64_t() const
{
  switch (type)
  {
    case vtInt:
      return vInt;

    case vtFloat:
      return (int64_t)vFloat;

    case vtString:
      return vStr.AsInt64();
  }
  return 0;
}

Property::operator int() const
{
  switch (type)
  {
    case vtInt:
      return (int)vInt;

    case vtFloat:
      return (int)vFloat;

    case vtString:
      return vStr.AsInt();
  }
  return 0;
}

Property::operator bool() const
{
  switch (type)
  {
    case vtInt:
      return vInt != 0;

    case vtFloat:
      return vFloat != 0;

    case vtString:
      return vStr == L"1";
  }
  return false;
}

Property::operator float() const
{
  switch (type)
  {
    case vtInt:
      return (float)vInt;

    case vtFloat:
      return vFloat;

    case vtString:
      return vStr.AsFloat();
  }
  return 0;
}

Property::operator const String() const
{
  switch (type)
  {
    case vtInt:
      return String(vInt);

    case vtFloat:
      return String(vFloat);

    case vtString:
      return vStr;
  }
  return L"";
}

bool Property::operator==(const Property & v) const
{
  switch (type)
  {
    case vtInt:
      return operator==((int)v);

    case vtFloat:
      return operator==((float)v);

    case vtString:
      return operator==(v.operator const String());
  }
  return false;
}
