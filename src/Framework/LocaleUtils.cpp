#include "LocaleUtils.h"

#include "StringVector.h"

void LoadLocale(const String & fn, Locale & locale)
{
  StringVector temp;
  if (temp.loadFromFile(fn))
  {
    for (size_t Index = 0; Index < temp.Count(); Index++)
    {
      const String & it = temp[Index];
      size_t p = it.find(L"=");
      if (p != (size_t)-1)
      {
        locale[it.substr(0, p).trim().trimquotes()] = it.substr(p + 1).trim().trimquotes();
      }
    }
  }
}
