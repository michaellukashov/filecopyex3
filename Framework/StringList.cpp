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
#include "lowlevelstr.h"
#include "stringlist.h"

StringList::StringList(void)
{
//   InitHeap();
//   SetBlock(4096);
//   SetOptions(0);
}

StringList::~StringList(void)
{
//  Clear();
}

// void StringList::SetBlock(int block)
// {
//   items.SetBlock(block/sizeof(ListItem));
// }

// int Compare(const StringList::ListItem& s1, const StringList::ListItem& s2,
//             const void* p)
// {
//   int res=_tcsncmp(
//     heap->LockString(s1.hnd), 
//     heap->LockString(s2.hnd),
//     p?(size_t)p:0x7FFFFFFF);
//   heap->Unlock(s1.hnd);
//   heap->Unlock(s2.hnd);
//   return res;
// }
// 
// int ICompare(const StringList::ListItem& s1, const StringList::ListItem& s2,
//              const void* p)
// {
//   int res=_tcsnicmp(
//     heap->LockString(s1.hnd), 
//     heap->LockString(s2.hnd),
//     p?(size_t)p:0x7FFFFFFF);
//   heap->Unlock(s1.hnd);
//   heap->Unlock(s2.hnd);
//   return res;
// }

// void StringList::SetOptions(int opt)
// {
//   items.SetSorted(opt & slSorted,
//     opt & slIgnoreCase ? ICompare:Compare);
// }

int StringList::Count()
{
  return items.Count();
}

const String StringList::operator[](int n)
{
  return items[n].str;
}

void StringList::Set(int n, const String& v)
{
	ListItem item;
	item.str = v;
	item.Data = 0;
	items[n] = item;
//   if (!items.IsSorted())
//     heap->ReallocString(items[n].hnd, v);
//   else
//   {
//     int val=Values(n);
//     Delete(n);
//     Add(v, val);
//   }
}

int& StringList::Values(int n)
{
  return items[n].Data;
}

void*& StringList::PtrValues(int n)
{
  return items[n].PtrData;
}

int StringList::Add(const String& v, int data)
{
  ListItem itm;
  itm.Data=data;
  itm.str=v;
  return items.Add(itm);
}

void StringList::Insert(int n, const String& v, int data)
{
  ListItem itm;
  itm.Data=data;
  itm.str=v;
  items.Insert(n, itm);
}

int StringList::Add(const String& v, void* data)
{
  ListItem itm;
  itm.PtrData=data;
  itm.str=v;
  return items.Add(itm);
}

void StringList::Insert(int n, const String& v, void* data)
{
  ListItem itm;
  itm.PtrData=data;
  itm.str=v;
  items.Insert(n, itm);
}

void StringList::Delete(int n)
{
//  heap->Free(items[n].hnd);
  items.Delete(n, 1);
}

void StringList::Clear()
{
//   for (int i=0; i<Count(); i++)
//     heap->Free(items[i].hnd);
  items.Clear();
}

void StringList::Exchange(int i, int j)
{
  items.Exchange(i, j);
}

void StringList::AddList(StringList &src)
{
  for (int i=0; i<src.Count(); i++)
    Add(src[i]);
}

int StringList::LoadFrom(FILE* f)
{
  Clear();
  union 
  {
    char c[2];
    wchar_t uc;
  } sign;
  int read=(int)fread(&sign, 1, 2, f);
  int unicode=(read==2) && (sign.uc==0xFEFF || sign.uc==0xFFFE);
  int inv=(unicode) && (sign.uc==0xFFFE);
  const int bsize=4096, ssize=4096;
  wchar_t buf[ssize];
  if (unicode)
  {
    const wchar_t CR='\r', LF='\n';
    wchar_t buffer[bsize], string[ssize];
    buffer[0]=sign.uc;
    int bpos=1, spos=0;
    while (1)
    {
      wchar_t oldc=0;
      read=(int)fread(buffer+bpos, sizeof(wchar_t), bsize-bpos, f);
      if (read<1) break;
      for (int i=bpos; i<read; i++)
      {
        if (inv) buffer[i]=((buffer[i]&0x00FF)<<8) | ((buffer[i]&0xFF00) >> 8);
        if (buffer[i]==CR || buffer[i]==LF && oldc!=CR) 
        {
          string[spos]=0;
          _wtotcs(buf, string, sizeof(buf)/sizeof(wchar_t));
          Add(buf);
          spos=0;
        }
        else if (buffer[i]!=CR && buffer[i]!=LF && spos<ssize)
          string[spos++]=buffer[i];
        oldc=buffer[i];
      }
      bpos=0;
    }
    if (spos) 
    {
      string[spos]=0;
      _wtotcs(buf, string, sizeof(buf)/sizeof(wchar_t));
      Add(buf);
    }
  }
  else
  {
    const char CR='\r', LF='\n';
    char buffer[bsize], string[ssize];
    int bpos=0, spos=0;
    if (read>=1) buffer[0]=sign.c[0];
    if (read>=2) buffer[1]=sign.c[1];
    bpos=read;
    while (1)
    {
      char oldc=0;
      read=(int)fread(buffer+bpos, sizeof(char), bsize-bpos, f);
      if (read<1) break;
      for (int i=0; i<read+bpos; i++)
      {
        if (buffer[i]==CR || buffer[i]==LF && oldc!=CR) 
        {
          string[spos]=0;
          _atotcs(buf, string, ssize);
          Add(buf);
          spos=0;
        }
        else if (buffer[i]!=CR && buffer[i]!=LF && spos<ssize)
          string[spos++]=buffer[i];
        oldc=buffer[i];
      }
      bpos=0;
    }
    if (spos) 
    {
      string[spos]=0;
      _atotcs(buf, string, ssize);
      Add(buf);
    }
  }
  return 1;
}

int StringList::SaveTo(FILE *f, TextFormat tf)
{
  if (tf==tfUnicode)
  {
    unsigned __int16 sign=0xFEFF;
    fwrite(&sign, 2, 1, f);
  }
  else if (tf==tfUnicodeBE)
  {
    unsigned __int16 sign=0xFFFE;
    fwrite(&sign, 2, 1, f);
  }
  for (int i=0; i<Count(); i++)
  {
    const wchar_t* s=(*this)[i].ptr();
    const int ssize=4096;
    if (tf!=tfANSI)
    {
      wchar_t buf[ssize];
      _ttowcs(buf, s, ssize);
      if (tf==tfUnicodeBE)
        for (int j=0; j<(int)wcslen(buf); j++)
          buf[j]=((buf[j]&0x00FF)<<8) | ((buf[j]&0xFF00) >> 8);
      fwrite(buf, sizeof(wchar_t), wcslen(buf), f);
      wcscpy_s(buf, ssize, L"\r\n");
      if (tf==tfUnicodeBE)
        for (int j=0; j<(int)wcslen(buf); j++)
          buf[j]=((buf[j]&0x00FF)<<8) | ((buf[j]&0xFF00) >> 8);
      fwrite(buf, sizeof(wchar_t), 2, f);
    }
    else
    {
      char buf[ssize];
      _ttoacs(buf, s, ssize);
      fwrite(buf, sizeof(char), strlen(buf), f);
      strcpy_s(buf, ssize, "\r\n");
      fwrite(buf, sizeof(char), 2, f);
    }
  }
  return 1;
}

int StringList::Load(const String& fn)
{
  FILE *f = NULL;
  _tfopen_s(&f, fn.ptr(), _T("rb"));
  if (!f) return 0;
  int res=LoadFrom(f);
  fclose(f);
  return res;
}

int StringList::Save(const String& fn, TextFormat tf)
{
  int attr=GetFileAttributes(fn.ptr());
  SetFileAttributes(fn.ptr(), FILE_ATTRIBUTE_NORMAL);
  FILE *f=NULL;
  _tfopen_s(&f, fn.ptr(), _T("wb"));
  int res;
  if (!f) res=0;
  else
  {
    res=SaveTo(f, tf);
    fclose(f);
  }
  if (attr != 0xFFFFFFFF)
    SetFileAttributes(fn.ptr(), attr);
  return res;
}

void StringList::LoadFromString(const String& s, wchar_t delim)
{
  LoadFromString(s.ptr(), delim);
}

void StringList::LoadFromString(const wchar_t* s, wchar_t delim)
{
  Clear();
  wchar_t *p=(wchar_t*)s, *pp=p, buf[4096];
  do
  {
    if (!*p || *p==delim)
    {
      int len=__min((int)(p-pp), 4095);
      _tcsncpy_s(buf, 4096, pp, len);
      buf[len]=0;
      pp=p+1;
      Add(buf);
    }
  } while (*p++);
}

int StringList::Find(const String& v, int start)
{
	for(int i = start; i < Count(); ++i)
	{
		if(items[i].str.cmp(v) == 0)
			return i;
	}
  return -1;
}

// int StringList::GFind(const String& v, int start)
// {
//   ListItem itm;
//   itm.str=v;
//   int res=items.GFind(itm, start, (const void*)0x7FFFFFFF);
//   return res;
// }
// 
// int StringList::PFind(const String& v, int start)
// {
//   ListItem itm;
//   itm.str=v;
//   int res=items.Find(itm, start, (const void*)(size_t)v.len());
//   return res;
// }
