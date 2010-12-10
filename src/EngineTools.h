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

#ifndef	__ENGINETOOLS_H__
#define	__ENGINETOOLS_H__

#pragma once

#define OPEN_BUF 128
#define OPEN_READ 1
#define OPEN_CREATE 2
#define OPEN_APPEND 4
#define OPEN_WRITE 8
#define OPEN_WRITE_BUF OPEN_WRITE|OPEN_BUF

#define LO32(i) (int)(i & 0xFFFFFFFF)
#define HI32(i) (int)(i >> 32)

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

void* Alloc(int size);
void Free(void *ptr);
void Compress(HANDLE handle, int f);
int GetCompression(HANDLE handle);
void Encrypt(const String& fn, int f);
void Encrypt(HANDLE handle, int f);
void *_ReadACL(const String& fn, SECURITY_INFORMATION si);
void  _WriteACL(const String& fn, SECURITY_INFORMATION si, void *buf);
void ReadACL(const String& fn, void **info1, void **info2);
void WriteACL(const String& fn, void *info1, void *info2);
void _CopyACL(const String& src, const String& dst, SECURITY_INFORMATION si);
void CopyACL(const String& src, const String& dst);
HANDLE Open(const String& fn, int mode, int attr=0);
void Close(HANDLE h);
__int64 FSeek(HANDLE h, __int64 pos, int method);
__int64 FTell(HANDLE h);
void SetFileSizeAndTime(const String& fn, __int64 size, FILETIME& time);
void SetFileSizeAndTime(HANDLE h, __int64 size, FILETIME& time);
void SetFileTime(const String& fn, FILETIME& time);
void SetFileTime(HANDLE h, FILETIME& time);
int Read(HANDLE h, void *buf, int size);
int Write(HANDLE h, void *buf, int size);
void ClearInput();
int GetSectorSize(const String& path);

#endif//__ENGINETOOLS_H__
