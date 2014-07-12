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

#ifndef __COMMON_H__
#define __COMMON_H__

#pragma once

#include "SDK/plugin.hpp"
#include "framework/objstring.h"

#define ATTR_OFF 0
#define ATTR_ON 1
#define ATTR_INHERIT 2

#define OM_CANCEL               -1
#define OM_PROMPT               0
#define OM_OVERWRITE            1
#define OM_SKIP                 2
#define OM_APPEND               3
#define OM_RENAME               4
#define OM_RESUME               5

#define FLG_COPIED 1
#define FLG_DELETED 2
#define FLG_SKIPPED 4
#define FLG_ERROR 8
#define FLG_SKIPNEWER 16
#define FLG_NEEDDEL 32
#define FLG_DECSIZE 64
#define FLG_DIR_PRE 256
#define FLG_DIR_POST 512
#define FLG_DESCFILE 1024
#define FLG_DESC_INVERSE 2048
#define FLG_DIR_NOREMOVE 4096
#define FLG_DIR_FORCE 8192
#define FLG_KEEPFILE 16384
#define FLG_BUFFERED 32768
#define FLG_TOP_DIR 65536

#define AF_CLEAR_RO 2
#define AF_STREAM 4
#define AF_TOPLEVEL 8
#define AF_DESCFILE 16
#define AF_DESC_INVERSE 32

#define VF_COMPRESSION 1
#define VF_ENCRYPTION 2
#define VF_RIGHTS 4
#define VF_STREAMS 8
#define VF_READONLY 16
#define VF_UNICODE 32
#define VF_CDROM 64

template <class T>
inline const T & Min(const T & a, const T & b) { return a < b ? a : b; }

template <class T>
inline const T & Max(const T & a, const T & b) { return a > b ? a : b; }

int ExistsN(const String & fn, int n);
String DupName(const String & src, int n);
int RmDir(const String & fn);
bool Newer(const FILETIME & ft1, const FILETIME & ft2);
bool Newer(const String & fn1, const FILETIME & ft2);
int Delete(const String & fn);

int VolFlags(const String & path);
int CheckParallel(const String & srcpath, const String & dstpath);

String GetSymLink(const String & dir);

void beep(int);

void DebugLog(const wchar_t * DebugMsg, ...);

String convertPath(enum CONVERTPATHMODES mode, String src);

inline int64_t GetTime()
{
  LARGE_INTEGER res;
  QueryPerformanceCounter(&res);
  return res.QuadPart;
}

inline int64_t TicksPerSec()
{
  LARGE_INTEGER res;
  QueryPerformanceFrequency(&res);
  return res.QuadPart;
}

#endif//__COMMON_H__
