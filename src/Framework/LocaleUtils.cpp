#include "LocaleUtils.h"

#include "StringVector.h"

void LoadLocale(const String & fn, Locale & locale)
{
  StringVector temp;
  if (temp.loadFromFile(fn))
  {
    for (size_t i = 0; i < temp.Count(); i++)
    {
      const String & it = temp[i];
      int p = it.find("=");
      if (p != -1)
      {
        locale[it.substr(0, p).trim().trimquotes()] = it.substr(p + 1).trim().trimquotes();
      }
    }
  }
}
