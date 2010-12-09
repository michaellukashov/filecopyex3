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
