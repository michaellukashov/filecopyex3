#pragma once
#include "../framework/objstring.h"
#include "../framework/valuelist.h"

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

extern LocaleList* Locale;

int ExistsN(const String& fn, int n);
String DupName(const String& src, int n);
int RmDir(const String& fn);
int Newer(FILETIME& ft1, FILETIME& ft2);
int Newer2(const String& fn1, FILETIME& ft2);
int Delete(const String& fn);

int VolFlags(const String& path);
int CheckParallel(const String& srcpath, const String& dstpath);

String MyGetLongPathName(const String& path);
String MyGetShortPathName(const String& path);
int FileHasUnicodeName(const String& fn);

String GetSymLink(const String &dir);

void beep();
void beep2();
void beep3();

void DebugLog(const wchar_t *DebugMsg, ...);
