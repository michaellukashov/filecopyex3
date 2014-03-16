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

//#include "framework/stdhdr.h"
#include "DescList.h"
#include "StdHdr.h"
#include "../tools.h"

DescList::DescList()
{
//  Names.SetOptions(slSorted | slIgnoreCase);
}

int DescList::LoadFromFile(const String& lfn)
{
	StringVector temp;
	if (!temp.loadFromFile(lfn)) {
		return false;
	}
	return LoadFromList(temp);
}

int DescList::LoadFromList(StringVector &list)
{
	String fn, desc;
	for (int i=0; i<list.Count(); i++) {
		String s = list[i];
		wchar_t c=s[0];
		if (c!='\t' && c!=' ' && c!='>' && c) {
			if (!fn.empty()) {
				names[fn] = Data(desc, 0);
				fn = "";
				desc = "";
			}
			if (c == '"') {
				int lf=s.substr(1).find('"');
				if (lf != -1) {
					fn = s.substr(1, lf).trim();
					desc = s.substr(lf+2).trim();

				}
			} else {
				int lf = s.find_first_of(" \t");
				if (lf != -1) {
					fn = s.substr(0, lf).trim();
					desc = s.substr(lf+1).trim();
				} else {
					fn = s;
					desc = "";
				}
			}
		} else {
			desc+=String("\n")+s;
		}
	}
	if (!fn.empty()) {
		names[fn] = Data(desc, 0);
	}
	return true;
}

#define dlNoMerge 1
#define dlNoSave 2

int DescList::SaveToFile(const String& fn)
{
	StringVector temp;
	for (DescListMap::iterator it = names.begin(); it != names.end(); it++ ) {
		if (!(it->second.flags & dlNoSave))	{
			String s;
			if (it->first.find(' ')) {
				s = String("\"") + it->first + "\"";
			} else {
				s = it->first;
			}
			s += String(" ") + it->second.desc;
			temp.AddString(s);
		}
	}
	if (temp.Count() > 0) {
		return temp.saveToFile(fn);
	}
	if (Delete(fn) || GetLastError() == ERROR_FILE_NOT_FOUND) {
		return true;
	}
	return false;
}

void DescList::Merge(DescList &add)
{
	for (DescListMap::iterator it = add.names.begin(); it != add.names.end(); it++ ) {
		if (!(it->second.flags & dlNoMerge))	{
			String name = it->first;
			names[name] = it->second;
		}
		}
}

void DescList::setFlag(const String& fn, int flag, int v)
{
	DescListMap::iterator it = names.find(fn);
	if (it != names.end()) {
		if (v) {
			it->second.flags &= ~flag;
		} else {
			it->second.flags |= flag;
		}
	}

}

void DescList::SetMergeFlag(const String& fn, int v)
{
	setFlag(fn, dlNoMerge, v);
}

void DescList::SetSaveFlag(const String& fn, int v)
{
	setFlag(fn, dlNoSave, v);
}

void DescList::setAllFlags(int flag, int v)
{
	for (DescListMap::iterator it = names.begin(); it != names.end(); it++ ) {
		if (v) {
			it->second.flags &= ~flag;
		} else {
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

/*
void DescList::Rename(int i, const String& dst, int changeName)
{
  String str=Values[Names.Values(i)].ltrim();
  if (str.substr(0, 1)=="\"")
    str=str.substr(str.substr(1).find("\"")+2);
  else
    str=str.substr(str.cfind(" \t\n"));
  Values.Set(Names.Values(i),
    ((dst.cfind(" ")!=-1)?(String("\"")+dst+"\""):(dst))+str);
  if (changeName) Names.Set(i, dst);
}
*/

void DescList::Rename(const String& src, const String& dst, int changeName)
{
	if (src != dst) {
		names[dst] = names[src];
		names.erase(src);
	}
}
