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

#include "Framework/StdHdr.h"
#include "Framework/ObjString.h"

#define msgw() 50

#define eeShowReopen 1
#define eeShowKeepFiles 2
#define eeYesNo 4
#define eeRetrySkipAbort 8
#define eerReopen 16
#define eerKeepFiles 32
#define eeOneLine 64
#define eeAutoSkipAll 128
#define eeAutoRetry 256

#define RES_ABORT 101
#define RES_YES 0
#define RES_NO -1

#define errfSkipAll 1024
#define errfKeepFiles 2048

void * Alloc(size_t size);
void Free(void * ptr);
void FCompress(HANDLE handle, uint32_t flags);
int FGetCompression(HANDLE handle);
void FEncrypt(const String & fn, uint32_t flags);
void FEncrypt(HANDLE handle, uint32_t flags);
void CopyACL(const String & src, const String & dst);
HANDLE FOpen(const String & fn, DWORD mode, DWORD attr);
int64_t FSeek(HANDLE h, int64_t pos, int method);
int64_t FTell(HANDLE h);
String GetLongFileName(const String & FileName);
void SetFileSizeAndTime2(const String & fn, int64_t size, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime);
void SetFileSizeAndTime2(HANDLE h, int64_t size, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime);
void SetFileTime2(const String & fn, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime);
void SetFileTime2(HANDLE h, FILETIME * creationTime, FILETIME * lastAccessTime, FILETIME * lastWriteTime);
size_t FRead(HANDLE h, void * buf, size_t size);
size_t FWrite(HANDLE h, void * buf, size_t size);
size_t GetSectorSize(const String & path);
