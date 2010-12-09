#pragma once

extern HINSTANCE hKernel, hAdvapi;

typedef
BOOL (WINAPI *_EncryptFile)(
  LPCTSTR lpFileName  // file name
);
extern _EncryptFile pEncryptFile;
typedef
BOOL (WINAPI *_DecryptFile)(
  LPCTSTR lpFileName,  // file name
  DWORD dwReserved     // reserved; must be zero
);
extern _DecryptFile pDecryptFile;

typedef
BOOL (WINAPI *_BackupRead)(
  HANDLE hFile,                // handle to file or directory
  LPBYTE lpBuffer,             // read buffer
  DWORD nNumberOfBytesToRead,  // number of bytes to read
  LPDWORD lpNumberOfBytesRead, // number of bytes read
  BOOL bAbort,                 // termination type
  BOOL bProcessSecurity,       // process security options
  LPVOID *lpContext            // context information
);
extern _BackupRead pBackupRead;
typedef
BOOL (WINAPI *_BackupWrite)(
  HANDLE hFile,                   // handle to file or directory
  LPBYTE lpBuffer,                // write buffer
  DWORD nNumberOfBytesToWrite,    // number of bytes to write
  LPDWORD lpNumberOfBytesWritten, // number of bytes written
  BOOL bAbort,                    // termination type
  BOOL bProcessSecurity,          // process security
  LPVOID *lpContext               // context information
);
extern _BackupWrite pBackupWrite;
typedef
BOOL (WINAPI *_BackupSeek)(
  HANDLE hFile,               // handle to file
  DWORD dwLowBytesToSeek,     // low-order DWORD of number of bytes
  DWORD dwHighBytesToSeek,    // high-order DWORD of number of bytes
  LPDWORD lpdwLowByteSeeked,  // number of bytes to forward
  LPDWORD lpdwHighByteSeeked, // number of bytes forwarded
  LPVOID *lpContext           // context information
);
extern _BackupSeek pBackupSeek;

typedef
BOOL (WINAPI *_OpenProcessToken)(
  HANDLE ProcessHandle, // handle to process
  DWORD DesiredAccess,  // desired access to process
  PHANDLE TokenHandle   // handle to open access token
);
extern _OpenProcessToken pOpenProcessToken;
typedef
BOOL (WINAPI *_LookupPrivilegeValue)(
  LPCTSTR lpSystemName,  // system name
  LPCTSTR lpName,        // privilege name
  PLUID lpLuid           // locally unique identifier
);
extern _LookupPrivilegeValue pLookupPrivilegeValue;
typedef
BOOL (WINAPI *_AdjustTokenPrivileges)(
  HANDLE TokenHandle,              // handle to token
  BOOL DisableAllPrivileges,       // disabling option
  PTOKEN_PRIVILEGES NewState,      // privilege information
  DWORD BufferLength,              // size of buffer
  PTOKEN_PRIVILEGES PreviousState, // original state buffer
  PDWORD ReturnLength              // required buffer size
);
extern _AdjustTokenPrivileges pAdjustTokenPrivileges;

typedef
BOOL (WINAPI *_GetFileSecurity)(
  LPCTSTR lpFileName,                        // file name
  SECURITY_INFORMATION RequestedInformation, // request
  PSECURITY_DESCRIPTOR pSecurityDescriptor,  // SD
  DWORD nLength,                             // size of SD
  LPDWORD lpnLengthNeeded                    // required buffer size
);
extern _GetFileSecurity pGetFileSecurity;
typedef
BOOL (WINAPI *_SetFileSecurity)(
  LPCTSTR lpFileName,                       // file name
  SECURITY_INFORMATION SecurityInformation, // contents
  PSECURITY_DESCRIPTOR pSecurityDescriptor  // SD
);
extern _SetFileSecurity pSetFileSecurity;

typedef
BOOL (WINAPI * _GetVolumeNameForVolumeMountPoint)(
  LPCTSTR lpszVolumeMountPoint,
  LPTSTR lpszVolumeName,
  DWORD cchBufferLength
);
extern _GetVolumeNameForVolumeMountPoint pGetVolumeNameForVolumeMountPoint;
typedef
BOOL (WINAPI * _GetVolumePathName)(
  LPCTSTR lpszFileName,
  LPTSTR lpszVolumePathName,
  DWORD cchBufferLength
);
extern _GetVolumePathName pGetVolumePathName;

#define IOCTL_VOLUME_BASE   ((DWORD) 'V')
#define IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS     \
  CTL_CODE(IOCTL_VOLUME_BASE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef DWORD (WINAPI * _GetLongPathName)(
  LPCTSTR lpszShortPath, // file name
  LPTSTR lpszLongPath,   // path buffer
  DWORD cchBuffer        // size of path buffer 
);
extern _GetLongPathName pGetLongPathName;
