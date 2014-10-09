/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010\nIdea & core: Max Antipin\nCoding: Serge Cheperis aka craZZy\nBugfixes: slst, CDK, Ivanych, Alter, Axxie and Nsky\nSpecial thanks to Vitaliy Tsubin\nFar 2 (32 & 64 bit) full unicode version by djdron

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
#include "DescList.h"
#include "FileUtils.h"

DescList::DescList()
{
}

bool DescList::LoadFromFile(const String & lfn)
{
  StringVector temp;
  if (!temp.loadFromFile(lfn))
  {
    return false;
  }
  return LoadFromList(temp);
}

bool DescList::LoadFromList(StringVector & list)
{
  String fn, desc;
  for (size_t Index = 0; Index < list.Count(); Index++)
  {
    String s = list[Index];
    wchar_t c = s[0];
    if (c != L'\t' && c != L' ' && c != L'>' && c)
    {
      if (!fn.empty())
      {
        names[fn] = Data(desc, 0);
        fn.Clear();
        desc.Clear();
      }
      if (c == L'"')
      {
        size_t lf = s.substr(1).find('"');
        if (lf != (size_t)-1)
        {
          fn = s.substr(1, lf).trim();
          desc = s.substr(lf + 2).trim();

        }
      }
      else
      {
        size_t lf = s.find_first_of(L" \t");
        if (lf != (size_t)-1)
        {
          fn = s.substr(0, lf).trim();
          desc = s.substr(lf + 1).trim();
        }
        else
        {
          fn = s;
          desc.Clear();
        }
      }
    }
    else
    {
      desc += String(L"\n") + s;
    }
  }
  if (!fn.empty())
  {
    names[fn] = Data(desc, 0);
  }
  return true;
}

#define dlNoMerge 1
#define dlNoSave 2

bool DescList::SaveToFile(const String & fn)
{
  StringVector temp;
  for (DescListMap::iterator it = names.begin(); it != names.end(); ++it)
  {
    if (!(it->second.flags & dlNoSave))
    {
      String s;
      if (it->first.find(' '))
      {
        s = String(L"\"") + it->first + L"\"";
      }
      else
      {
        s = it->first;
      }
      s += String(L" ") + it->second.desc;
      temp.AddString(s);
    }
  }
  if (temp.Count() > 0)
  {
    return temp.saveToFile(fn);
  }
  if (FDelete(fn) || ::GetLastError() == ERROR_FILE_NOT_FOUND)
  {
    return true;
  }
  return false;
}

void DescList::Merge(DescList & add)
{
  for (DescListMap::iterator it = add.names.begin(); it != add.names.end(); ++it)
  {
    if (!(it->second.flags & dlNoMerge))
    {
      String name = it->first;
      names[name] = it->second;
    }
  }
}

void DescList::setFlag(const String & fn, uint32_t flag, uint32_t v)
{
  DescListMap::iterator it = names.find(fn);
  if (it != names.end())
  {
    if (v)
    {
      it->second.flags &= ~flag;
    }
    else
    {
      it->second.flags |= flag;
    }
  }
}

void DescList::SetMergeFlag(const String & fn, uint32_t v)
{
  setFlag(fn, dlNoMerge, v);
}

void DescList::SetSaveFlag(const String & fn, uint32_t v)
{
  setFlag(fn, dlNoSave, v);
}

void DescList::setAllFlags(uint32_t flag, uint32_t v)
{
  for (DescListMap::iterator it = names.begin(); it != names.end(); ++it)
  {
    if (v)
    {
      it->second.flags &= ~flag;
    }
    else
    {
      it->second.flags |= dlNoMerge;
    }
  }
}

void DescList::SetAllMergeFlags(int v)
{
  setAllFlags(dlNoMerge, v);
}

void DescList::SetAllSaveFlags(int v)
{
  setAllFlags(dlNoSave, v);
}

void DescList::Rename(const String & src, const String & dst, int changeName)
{
  if (src != dst)
  {
    names[dst] = names[src];
    names.erase(src);
  }
}
