#ifndef __LOCALELIST_H__
#define __LOCALELIST_H__

#include <map>

#include "ObjString.h"

typedef std::map<const String,String> Locale;
void LoadLocale(const String& fn, Locale &locale);

#endif