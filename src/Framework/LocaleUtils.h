#pragma once

#include <map>

#include "ObjString.h"

typedef std::map<const String,String> Locale;
void LoadLocale(const String & fn, Locale & locale);

