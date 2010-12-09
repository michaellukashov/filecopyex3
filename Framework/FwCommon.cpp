#include <stdhdr.h>
#include "fwcommon.h"
#include "objstring.h"

void FWError(const TCHAR *s)
{
  if (errorHandler)
    errorHandler(s);
  else
  {
    MessageBox(0, s, _T("FileCopyEx plugin error"), MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
    DebugBreak();
  }
}

void FWError(const String &s)
{
  FWError(s.ptr());
}

ErrorHandler errorHandler=NULL;

extern void InitObjMgr();
int WinNT, Win2K, WinXP, Win98, WinNT4;
HANDLE hInstance;

TCHAR* AWErrMsgWinNT = _T("Remove FileCopyExA.dll file from plugins folder!\n\
it's intended to use on Win9x based operating systems.");
TCHAR* AWErrMsgWin98 = _T("Remove FileCopyExW.dll file from plugins folder!\n\
it's intended to use on WinNT based operating systems.");

BOOL __stdcall DllMain(HANDLE hInst, ULONG reason, LPVOID)
{
  hInstance=hInst;
  if (reason==DLL_PROCESS_ATTACH)
  {
    OSVERSIONINFO osv;
    osv.dwOSVersionInfoSize=sizeof(osv);
    GetVersionEx(&osv);
    WinNT=osv.dwPlatformId==VER_PLATFORM_WIN32_NT;
    WinNT4=WinNT && osv.dwMajorVersion==4;
    Win2K=WinNT && osv.dwMajorVersion>=5;
    WinXP=Win2K && (osv.dwMajorVersion>5 || osv.dwMinorVersion>=1);
    Win98=osv.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS 
      && osv.dwMinorVersion>0;
#ifdef SEPARATE_UNICODE
  #ifdef UNICODE
      if (!WinNT)
            {
                FWError(AWErrMsgWin98);
                return FALSE;
            }
  #else
      if (WinNT)
            {
                FWError(AWErrMsgWinNT);
                return FALSE;
            }
  #endif
#endif
    InitHeap();
    InitObjMgr();
  }
  return TRUE;
}
