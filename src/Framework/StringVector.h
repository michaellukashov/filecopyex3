#pragma once

#include <vector>

#include "StringParent.h"

class StringVector : public StringParent
{
public:
  StringVector() {}
  virtual ~StringVector() {}

  virtual void Clear() { data.clear(); }
  virtual const String & operator[](size_t i) const { return data[i]; }
  virtual void AddString(const String & v) { data.push_back(v); }
  virtual size_t Count() const { return data.size(); }
  intptr_t Find(const String & v, intptr_t start = 0) const
  {
    for (size_t Index = start; Index < Count(); ++Index)
    {
      if (data[Index].cmp(v) == 0)
        return Index;
    }
    return -1;
  }

  intptr_t FindAny(const String & v, intptr_t start = 0) const
  {
    for (size_t Index = start; Index < Count(); ++Index)
    {
      if (data[Index].find(v) != size_t(-1))
        return Index;
    }
    return -1;
  }

private:
  std::vector<String> data;
};

