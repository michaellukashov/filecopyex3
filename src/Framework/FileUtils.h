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

#include "StdHdr.h"
#include "ObjString.h"

#define MAX_FILENAME 4096
#define DEFAULT_SECTOR_SIZE 4096

#define OPEN_BUF 128
#define OPEN_READ 1
#define OPEN_CREATE 2
#define OPEN_APPEND 4
#define OPEN_WRITE 8
#define OPEN_WRITE_BUF OPEN_WRITE|OPEN_BUF

#define LO32(i) (int)(i & 0xFFFFFFFF)
#define HI32(i) (int)(i >> 32)

String ExtractFileName(const String &);
String ExtractFilePath(const String &);
String ExtractFileExt(const String &);
String CutEndSlash(const String &);
String AddEndSlash(const String &);
String ChangeFileExt(const String &, const String &);

#define gslExpandSubst 1
#define gslExpandNetMappings 2
#define gslExpandReparsePoints 4
#define gslExpandMountPoints 8

#define rfnNoNetExpand 1

extern HANDLE Open(const String & fn, DWORD mode, DWORD attr);
void Close(HANDLE h);
int Delete(const String & fn);

String GetFileRoot(const String &);
String GetRealFileName(const String &, int flg = 0);
bool GetSymLink(const String & dir, String & res, int flg);
String GetFileNameRoot(const String &);
String ExpandEnv(const String &);

String ApplyFileMask(const String & name, const String & mask);
String ApplyFileMaskPath(const String & name, const String & mask);

bool FileExists(const String & name);
bool MoveFile(const String & src, const String & dst, intptr_t replace);
void ForceDirectories(const String & s);

int64_t FileSize(HANDLE h);
int64_t FileSize(const String & fn);

String TempName();
String TempPath();
String TempPathName();

void Out(const String & s);

bool GetPrimaryVolumeMountPoint(const String & VolumeMountPointForPath, String & PrimaryVolumeMountPoint);

