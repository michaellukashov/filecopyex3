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

#include "../stdhdr.h"
#include "../lowlevelstr.h"
#include "farplugin.h"

FarRegistry::FarRegistry(void)
{
}

FarRegistry::~FarRegistry(void)
{
}

String FarRegistry::ResolveKey(const String& s)
{
  if (s!="" && s[0]=='\\')
    return s.right(s.len()-1);
  else if (s!="")
    return PluginRootKey+"\\"+s;
  else
    return PluginRootKey;
}

String FarRegistry::GetString(const String& key, const String& value, const String& def)
{
  HKEY hKey=OpenKey(key);
  if (hKey)
  {
    DWORD Type;
    DWORD Size;
    if (RegQueryValueEx(hKey, value.ptr(), 0, &Type, NULL, &Size)==0
      && (Type==REG_SZ || Type==REG_EXPAND_SZ))
    {
      String res(' ', Size/sizeof(wchar_t));
      if (RegQueryValueEx(hKey, value.ptr(), 0, &Type, (LPBYTE)(wchar_t*)res.ptr(), 
        &Size)==0)
      {
        RegCloseKey(hKey);
        return res;
      }
    }
    RegCloseKey(hKey);
  }
  return def;
}

void FarRegistry::SetString(const String& key, const String& value, const String& buf)
{
  HKEY hKey=CreateKey(key);
  if (hKey)
  {
    RegSetValueEx(hKey, value.ptr(), 0, REG_SZ, (LPBYTE)(wchar_t*)buf.ptr(), 
      sizeof(wchar_t)*(buf.len()+1));
    RegCloseKey(hKey);
  }
}

int FarRegistry::GetInt(const String& key, const String& value, int def)
{
  HKEY hKey=OpenKey(key);
  if (hKey)
  {
    DWORD Type, Res, Size=sizeof(DWORD);
    int res=RegQueryValueEx(hKey, value.ptr(), 0, &Type, (LPBYTE)&Res, &Size);
    RegCloseKey(hKey);
    if (res || Type!=REG_DWORD || Size!=sizeof(DWORD)) return def;
    else return Res;
  }
  else return def;
}

void FarRegistry::SetInt(const String& key, const String& value, int data)
{
  HKEY hKey=CreateKey(key);
  if (hKey)
  {
    RegSetValueEx(hKey, value.ptr(), 0, REG_DWORD, (LPBYTE)&data, sizeof(data));
    RegCloseKey(hKey);
  }
}

void FarRegistry::ReadList(const String& key, StringList& list)
{
  list.Clear();
  HKEY hKey=OpenKey(key);
  if (hKey)
  {
    int c=0;
    wchar_t Name[512], Data[4096];
    DWORD NameSize=512, DataSize=sizeof(Data), Type;
    while (RegEnumValue(hKey, c++, Name, &NameSize, NULL, &Type, 
      (LPBYTE)Data, &DataSize)==0)
    {
      if (Type==REG_SZ || Type==REG_EXPAND_SZ)
        list.Add(Data);
      NameSize=512;
      DataSize=sizeof(Data);
    }
    RegCloseKey(hKey);
  }
}

void FarRegistry::WriteList(const String& key, StringList& list)
{
  HKEY hKey=CreateKey(key);
  if (hKey)
  {
    int c=0;
    wchar_t Name[512];
    DWORD NameSize=512, Type;
    StringList ValueList;
    while (RegEnumValue(hKey, c++, Name, &NameSize, NULL, &Type, NULL, NULL)==0)
    {
      if (Type==REG_SZ || Type==REG_EXPAND_SZ)
        ValueList.Add(Name);
      NameSize=512;
    }
    for (int i=0; i<ValueList.Count(); i++)
      RegDeleteValue(hKey, ValueList[i].ptr());
    c=0;
    for (int i=0; i<list.Count(); i++)
    {
      const String& data=list[i];
      wchar_t Name[16];
      _itot_s(c++, Name, 16, 10);
      RegSetValueEx(hKey, Name, 0, REG_SZ, (LPBYTE)(wchar_t*)data.ptr(), 
        sizeof(wchar_t)*((int)data.len()+1));
    }
    RegCloseKey(hKey);
  }
}

void FarRegistry::DeleteKey(const String& key)
{
  RegDeleteKey(HKEY_CURRENT_USER, ResolveKey(key).ptr());
}

void FarRegistry::CopyKey(const String& Src, const String& Dst)
{
  HKEY hSrc=OpenKey(Src);
  if (!hSrc) return;
  HKEY hDst=CreateKey(Dst);
  if (!hDst) 
  {
    RegCloseKey(hSrc);
    return;
  }
  int i=0;
  DWORD dcb, ncb, type;
  wchar_t name[512];
  BYTE data[4096];
  while (1)
  {
    dcb=sizeof(data);
    ncb=sizeof(name)/sizeof(wchar_t);
    if (RegEnumValue(hSrc, i++, name, &ncb, NULL, &type, data, &dcb)) break;
    RegSetValueEx(hDst, name, NULL, type, data, dcb);
  }
  RegCloseKey(hSrc);
  RegCloseKey(hDst);
}

HKEY FarRegistry::OpenKey(const String& key)
{
  HKEY hKey;
  if (RegOpenKeyEx(HKEY_CURRENT_USER, ResolveKey(key).ptr(), 0, 
    KEY_QUERY_VALUE, &hKey)==0) 
    return hKey;
  else 
    return 0;
}

HKEY FarRegistry::CreateKey(const String& key)
{
  HKEY hKey;
  DWORD Disposition;
  if (RegCreateKeyEx(HKEY_CURRENT_USER, ResolveKey(key).ptr(), 0, 
    NULL, 0, KEY_WRITE | KEY_QUERY_VALUE, NULL,
    &hKey, &Disposition)==0) 
    return hKey;
  else 
    return 0;
}


FarRegistry Registry;