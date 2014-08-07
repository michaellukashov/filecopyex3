#pragma once

#include "StdHdr.h"
#include "ObjString.h"

enum TextFormat { tfOEM, tfUnicode, tfUnicodeBE };

class StringParent
{
public:
  StringParent() {}
  virtual ~StringParent() {}

  virtual void Clear() = 0;
  virtual const String & operator[](size_t) const = 0;
  virtual void AddString(const String &) = 0;
  virtual size_t Count() const = 0;

  int loadFromFile(FILE * f);
  int loadFromFile(const String & fileName);
  void loadFromString(const String &, wchar_t delim);
  void loadFromString(const wchar_t *, wchar_t delim);

  bool saveToFile(FILE * f, TextFormat tf = tfOEM);
  bool saveToFile(const String & fileName, TextFormat tf = tfOEM);
};

