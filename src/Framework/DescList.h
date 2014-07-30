#pragma once

#include <map>

#include "StringVector.h"

class DescList
{
public:
  DescList();
  bool LoadFromFile(const String & fn);
  void Merge(DescList &);
  bool SaveToFile(const String & fn);
  void SetAllMergeFlags(int v);
  void SetAllSaveFlags(int v);
  void SetMergeFlag(const String &, uint32_t v);
  void SetSaveFlag(const String &, uint32_t v);
  void Rename(const String & src, const String & dst, int changeName = 0);

private:
  void setFlag(const String &, uint32_t flag, uint32_t v);
  void setAllFlags(uint32_t flag, uint32_t v);

  class Data
  {
  public:
    String desc;
    uint32_t flags;

    Data() : flags(0) {};
    Data(const String & _desc, int _flags = 0): desc(_desc), flags(_flags) {};
  };
  typedef std::map<String, Data> DescListMap;
  DescListMap names;

  bool LoadFromString(wchar_t * ptr);
  bool LoadFromList(StringVector & list);
};
