#include <stdhdr.h>
#include "api.h"

HINSTANCE hKernel = GetModuleHandle(_T("kernel32.dll")),
  hAdvapi = GetModuleHandle(_T("advapi32.dll"));

#ifdef UNICODE
  _EncryptFile pEncryptFile = 
    (_EncryptFile)GetProcAddress(hAdvapi, "EncryptFileW");
  _DecryptFile pDecryptFile = 
    (_DecryptFile)GetProcAddress(hAdvapi, "DecryptFileW");
#else
  _EncryptFile pEncryptFile = 
    (_EncryptFile)GetProcAddress(hAdvapi, "EncryptFileA");
  _DecryptFile pDecryptFile = 
    (_DecryptFile)GetProcAddress(hAdvapi, "DecryptFileA");
#endif

_BackupRead pBackupRead = 
  (_BackupRead)GetProcAddress(hKernel, "BackupRead");
_BackupWrite pBackupWrite = 
  (_BackupWrite)GetProcAddress(hKernel, "BackupWrite");
_BackupSeek pBackupSeek = 
  (_BackupSeek)GetProcAddress(hKernel, "BackupSeek");

_OpenProcessToken pOpenProcessToken = 
  (_OpenProcessToken)GetProcAddress(hAdvapi, "OpenProcessToken");
_AdjustTokenPrivileges pAdjustTokenPrivileges = 
  (_AdjustTokenPrivileges)GetProcAddress(hAdvapi, "AdjustTokenPrivileges");
#ifdef UNICODE
  _LookupPrivilegeValue pLookupPrivilegeValue = 
    (_LookupPrivilegeValue)GetProcAddress(hAdvapi, "LookupPrivilegeValueW");
#else
  _LookupPrivilegeValue pLookupPrivilegeValue = 
    (_LookupPrivilegeValue)GetProcAddress(hAdvapi, "LookupPrivilegeValueA");
#endif

#ifdef UNICODE
  _GetFileSecurity pGetFileSecurity = 
    (_GetFileSecurity)GetProcAddress(hAdvapi, "GetFileSecurityW");
  _SetFileSecurity pSetFileSecurity = 
    (_SetFileSecurity)GetProcAddress(hAdvapi, "SetFileSecurityW");
#else
  _GetFileSecurity pGetFileSecurity = 
    (_GetFileSecurity)GetProcAddress(hAdvapi, "GetFileSecurityA");
  _SetFileSecurity pSetFileSecurity = 
    (_SetFileSecurity)GetProcAddress(hAdvapi, "SetFileSecurityA");
#endif

#ifdef UNICODE
  _GetVolumeNameForVolumeMountPoint pGetVolumeNameForVolumeMountPoint = 
    (_GetVolumeNameForVolumeMountPoint)GetProcAddress(hKernel, "GetVolumeNameForVolumeMountPointW");
  _GetVolumePathName pGetVolumePathName =
    (_GetVolumePathName)GetProcAddress(hKernel, "GetVolumePathNameW");
#else
  _GetVolumeNameForVolumeMountPoint pGetVolumeNameForVolumeMountPoint = 
    (_GetVolumeNameForVolumeMountPoint)GetProcAddress(hKernel, "GetVolumeNameForVolumeMountPointA");
  _GetVolumePathName pGetVolumePathName =
    (_GetVolumePathName)GetProcAddress(hKernel, "GetVolumePathNameA");
#endif

#ifdef UNICODE
  _GetLongPathName pGetLongPathName =
    (_GetLongPathName)GetProcAddress(hKernel, "GetLongPathNameW");
#else
  _GetLongPathName pGetLongPathName =
    (_GetLongPathName)GetProcAddress(hKernel, "GetLongPathNameA");
#endif
