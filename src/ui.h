/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010
Idea & core: Max Antipin
Coding: Serge Cheperis aka craZZy
Bugfixes: slst, CDK, Ivanych, Alter, Axxie and Nsky
Special thanks to Vitaliy Tsubin
Far 2 (32 & 64 bit) full unicode version by djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "ObjString.h"

intptr_t ShowMessage(const String &, const String &, int);
intptr_t ShowMessageOK(const String &, const String &);
intptr_t ShowMessageHelp(const String &, const String &, int, const String &);
intptr_t ShowMessageEx(const String &, const String &, const String &, int);
intptr_t ShowMessageExHelp(const String &, const String &, const String &, int, const String &);

#define RES_RETRY 1
#define RES_SKIP 0

void Error(const String &, int code);
void Error2(const String &, const String &, int code);
intptr_t Error2RS(const String &, const String &, int code);

String GetErrText(int code);

String FormatWidth(const String &, intptr_t);
String FormatWidthNoExt(const String &, intptr_t);
String SplitWidth(const String &, intptr_t);
