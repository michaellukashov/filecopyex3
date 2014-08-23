#pragma once

#include "Framework/ObjString.h"

void Bind(const String & key, const String & code, const String & desc, int id);
int Binded(const String & key);
void Unbind(const String & key);

