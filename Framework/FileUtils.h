/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010 Serge Cheperis aka craZZy
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

#include "objstring.h"
#include "properties.h"

#define MAX_FILENAME 4096

String ExtractFileName(const String&);
String ExtractFilePath(const String&);
String ExtractFileExt(const String&);
String CutEndSlash(const String&);
String AddEndSlash(const String&);
String ChangeFileExt(const String&, const String&);

#define gslExpandSubst 1
#define gslExpandNetMappings 2
#define gslExpandReparsePoints 4
#define gslExpandMountPoints 8

#define rfnNoNetExpand 1

String GetFileRoot(const String&);
String GetRealFileName(const String&, int flg=0);
int GetSymLink(const String &dir, String &res, int flg);
String GetFileNameRoot(const String&);
String ExpandEnv(const String&);

String ApplyFileMask(const String& name, const String& mask);
String ApplyFileMaskPath(const String& name, const String& mask);

int FileExists(const String& name);
int MoveFile(const String& src, const String& dst, int replace);
void ForceDirectories(const String& s);

__int64 FileSize(HANDLE h);
__int64 FileSize(const String& fn);

String TempName();
String TempPath();
String TempPathName();

void Out(const String &s);

BOOL GetPrimaryVolumeMountPoint(const String& VolumeMountPointForPath,
                                String& PrimaryVolumeMountPoint);
