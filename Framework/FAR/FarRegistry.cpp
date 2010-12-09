#include <stdhdr.h>
#include "lowlevelstr.h"
#include "far/farplugin.h"

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
      String res(' ', Size/sizeof(TCHAR));
      if (RegQueryValueEx(hKey, value.ptr(), 0, &Type, (LPBYTE)(TCHAR*)res.ptr(), 
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
    RegSetValueEx(hKey, value.ptr(), 0, REG_SZ, (LPBYTE)(TCHAR*)buf.ptr(), 
      sizeof(TCHAR)*(buf.len()+1));
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
    TCHAR Name[512], Data[4096];
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
    TCHAR Name[512];
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
    for (i=0; i<list.Count(); i++)
    {
      const String& data=list[i];
      TCHAR Name[16];
      _itot(c++, Name, 10);
      RegSetValueEx(hKey, Name, 0, REG_SZ, (LPBYTE)(TCHAR*)data.ptr(), 
        sizeof(TCHAR)*((int)data.len()+1));
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
  TCHAR name[512];
  BYTE data[4096];
  while (1)
  {
    dcb=sizeof(data);
    ncb=sizeof(name)/sizeof(TCHAR);
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