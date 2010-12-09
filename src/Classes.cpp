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

#include "../framework/stdhdr.h"
#include "classes.h"
#include "common.h"
#include "../framework/fwcommon.h"

wchar_t* FileNameStoreEnum::GetNext()
{
  if (Cur>=Store->Count()) return NULL;
  wchar_t *ptr=(*Store)[Cur++];
  if (*ptr=='+') 
  {
    wchar_t *p=CurPath;
    while (*p) p++;
    if (p>CurPath) *p++='\\';
    _tcopy(p, ptr+1, MAX_FILENAME-(p-CurPath));
    _tcscpy_s(Buffer, MAX_FILENAME, CurPath);
  }
  else if (*ptr=='-')
  {
    _tcscpy_s(Buffer, MAX_FILENAME, CurPath);
    wchar_t *p=CurPath;
    while (*p) p++;
    p--;
    while (p>CurPath && *p!='\\') p--;
    *p=0;
  }
  else if (*ptr=='*')
  {
    _tcscpy_s(Buffer, MAX_FILENAME, CurPath);
    CurPath[0]=0;
  }
  else
  {
    wchar_t *p=CurPath, *b=Buffer;
    while (*p) *b++=*p++;
    if (b>Buffer) *b++='\\';
    _tcopy(b, ptr+1, MAX_FILENAME-(b-Buffer));
  }
  return Buffer;
}

void FileNameStoreEnum::Skip()
{
  if (Cur>=Store->Count()) return;
  wchar_t *ptr=(*Store)[Cur++];
  if (*ptr=='+') 
  {
    wchar_t *p=CurPath;
    while (*p) p++;
    if (p>CurPath) *p++='\\';
    _tcopy(p, ptr+1, MAX_FILENAME-(p-CurPath));
  }
  else if (*ptr=='-')
  {
    wchar_t *p=CurPath;
    while (*p) p++;
    p--; 
    while (p>CurPath && *p!='\\') p--;
    *p=0;
  }
  else if (*ptr=='*')
  {
    CurPath[0]=0;
  }
}

void FileNameStoreEnum::ToFirst()
{
  Cur=0;
  Buffer[0]=CurPath[0]=0;
}

wchar_t* FileNameStoreEnum::GetByNum(int n)
{
  if (n<Cur-1) 
  {
    FWError(_T("FileNameStoreEnum::GetByNum - assertion failure :))"));
    return NULL;
  }
  else if (n==Cur-1)
    return Buffer;
  else
  {
    while (Cur<n) Skip();
    return GetNext();
  }
}

wchar_t* FileNameStore::GetNameByNum(int n)
{
  return (*this)[n]+1;
}

int FileNameStore::AddRel(wchar_t pc, wchar_t* s)
{
  wchar_t buf[MAX_FILENAME+1];
  buf[0]=pc;
  _tcopy(buf+1, s, MAX_FILENAME);
  return Add(buf);
}

DescList::DescList()
{
//  Names.SetOptions(slSorted | slIgnoreCase);
}

int DescList::LoadFromFile(const String& lfn)
{
  StringList temp;
  if (!temp.Load(lfn)) return FALSE;
  return LoadFromList(temp);
}

int DescList::LoadFromString(wchar_t *ptr)
{
  StringList temp;
  temp.LoadFromString(ptr, '\n');
  return LoadFromList(temp);
}

int DescList::LoadFromList(StringList &list)
{
  String fn, desc;
  for (int i=0; i<list.Count(); i++)
  {
    String s=list[i];
    wchar_t c=s[0];
    if (c!='\t' && c!=' ' && c!='>' && c)
    {
      if (fn!="")
      {
        Names.Add(fn, Values.Add(desc));
        fn="";
      }
      if (c=='"') 
      {
        int lf=s.substr(1).cfind('"');
        fn=s.substr(1, lf);
      }
      else
      {
        int lf=s.cfind(" \t");
        fn=s.substr(0, lf);
      }
      desc=s;
    }
    else desc+=String("\n")+s;
  }
  if (fn!="") Names.Add(fn, Values.Add(desc));
  return TRUE;
}

#define dlNoMerge 1
#define dlNoSave 2

int DescList::SaveToFile(const String& fn)
{
  StringList temp;
  for (int i=0; i<Values.Count(); i++)
    if (!(Values.Values(i) & dlNoSave))
    {
      String s=Values[i];
      temp.Add(s);
    }
  if (temp.Count()>0)
    return temp.Save(fn);
  else if (Delete(fn) || GetLastError()==ERROR_FILE_NOT_FOUND) return TRUE;
  else return FALSE;
}

void DescList::Merge(DescList &list)
{
  for (int i=0; i<list.Count(); i++)
    if (!(list.Flags(i) & dlNoMerge))
    {
      String n=list.Name(i);
      int j=Names.Find(n);
      if (j!=-1) Values.Set(Names.Values(j), list.Value(i));
      else Names.Add(n, Values.Add(list.Value(i)));
    }
}

void DescList::SetMergeFlag(const String& fn, int flag)
{
  int j=Names.Find(fn);
  if (j!=-1) 
    if (flag) Flags(j)&=~dlNoMerge;
    else Flags(j)|=dlNoMerge;
}

void DescList::SetSaveFlag(const String& fn, int flag)
{
  int j=Names.Find(fn);
  if (j!=-1) 
    if (flag) Flags(j)&=~dlNoSave;
    else Flags(j)|=dlNoSave;
}

void DescList::SetAllMergeFlags(int flag)
{
  for (int j=0; j<Count(); j++)
    if (flag) Flags(j)&=~dlNoMerge;
    else Flags(j)|=dlNoMerge;
}

void DescList::SetAllSaveFlags(int flag)
{
  for (int j=0; j<Count(); j++)
    if (flag) Flags(j)&=~dlNoSave;
    else Flags(j)|=dlNoSave;
}

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

void DescList::Rename(const String& src, const String& dst, int changeName)
{
  // bugs #42, #43 fixed by axxie
  if (dst.icmp(src))
  {
    int j=Names.Find(dst);
    if (j!=-1) 
    {
      Names.Delete(j);
      Values.Delete(j);
    }
  }
  int i=Names.Find(src);
  if (i!=-1) Rename(i, dst, changeName);
}

int DescList::Add(const String& name, const String& val)
{
  int i=Names.Find(name);
  if (i==-1) return Names.Add(name, Values.Add(val));
  else 
  {
    Values.Set(Names.Values(i), val);
    return i;
  }
}

String DescList::Value(const String& name)
{
  int i=Names.Find(name);
  if (i==-1) return "";
  else return Values[Names.Values(i)];
}
