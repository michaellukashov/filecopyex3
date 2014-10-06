#pragma once

#include <map>

#include "StdHdr.h"
#include "ObjString.h"

typedef std::map<const String, String> Locale;
void LoadLocale(const String & fn, Locale & locale);
