#pragma once
#include "../stringlist.h"

class FarRegistry
{
public:
  FarRegistry(void);
  virtual ~FarRegistry(void);
  int GetInt(const String&, const String&, int);
  void SetInt(const String&, const String&, int);
  String GetString(const String&, const String&, const String&);
  void SetString(const String&, const String&, const String&);
  void ReadList(const String&, StringList&);
  void WriteList(const String&, StringList&);
  void DeleteKey(const String&);
  void CopyKey(const String&, const String&);
protected:
  String ResolveKey(const String&);
  HKEY OpenKey(const String&);
  HKEY CreateKey(const String&);
};

extern FarRegistry Registry;
