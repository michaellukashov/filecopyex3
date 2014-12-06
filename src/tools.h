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

#include "SDK/plugin.hpp"
#include "framework/ObjString.h"

#define ATTR_OFF 0
#define ATTR_ON 1
#define ATTR_INHERIT 2

enum TOverwriteMode
{
  OM_CANCEL =             -1,
  OM_PROMPT =             0,
  OM_OVERWRITE =          1,
  OM_SKIP =               2,
  OM_APPEND =             3,
  OM_RENAME =             4,
  OM_RESUME =             5,
};

const int FLG_COPIED = 1;
const int FLG_DELETED = 2;
const int FLG_SKIPPED = 4;
const int FLG_ERROR = 8;
const int FLG_SKIPNEWER = 16;
const int FLG_NEEDDEL = 32;
const int FLG_DECSIZE = 64;
const int FLG_DIR_PRE = 256;
const int FLG_DIR_POST = 512;
const int FLG_DESCFILE = 1024;
const int FLG_DESC_INVERSE = 2048;
const int FLG_DIR_NOREMOVE = 4096;
const int FLG_DIR_FORCE = 8192;
const int FLG_KEEPFILE = 16384;
const int FLG_BUFFERED = 32768;
const int FLG_TOP_DIR = 65536;

const int AF_CLEAR_RO = 2;
const int AF_STREAM = 4;
const int AF_TOPLEVEL = 8;
const int AF_DESCFILE = 16;
const int AF_DESC_INVERSE = 32;

const int VF_COMPRESSION = 1;
const int VF_ENCRYPTION = 2;
const int VF_RIGHTS = 4;
const int VF_FAT = 8;
const int VF_STREAMS = 16;
const int VF_READONLY = 32;
const int VF_UNICODE = 64;
const int VF_CDROM = 128;

template <class T>
inline const T & Min(const T & a, const T & b) { return a < b ? a : b; }

template <class T>
inline const T & Max(const T & a, const T & b) { return a > b ? a : b; }

bool ExistsN(const String & fn, intptr_t n);
String DupName(const String & src, intptr_t n);
bool RmDir(const String & fn);
bool Newer(const FILETIME & ft1, const FILETIME & ft2);
bool Newer(const String & fn1, const FILETIME & ft2);

uint32_t VolFlags(const String & Path);
intptr_t CheckParallel(const String & srcpath, const String & dstpath);

String GetSymLink(const String & dir);

void Beep(int);

void DebugLog(const wchar_t * DebugMsg, ...);

String ConvertPath(enum CONVERTPATHMODES mode, const String & src);

inline int64_t GetTime()
{
  LARGE_INTEGER res;
  ::QueryPerformanceCounter(&res);
  return res.QuadPart;
}

inline int64_t TicksPerSec()
{
  LARGE_INTEGER res;
  ::QueryPerformanceFrequency(&res);
  return res.QuadPart;
}
