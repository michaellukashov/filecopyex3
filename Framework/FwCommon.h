#pragma once
#include <tchar.h>

typedef void (*ErrorHandler)(const TCHAR*);

class String;

void FWError(const String&);
void FWError(const TCHAR*);
extern ErrorHandler errorHandler;

#define MAKEINT64(low, high) ((((__int64)(high))<<32)|((__int64)low))

extern int WinNT, Win2K, WinXP, Win98, WinNT4;
extern HANDLE hInstance;

BOOL __stdcall DllMain(HANDLE hInst, ULONG reason, LPVOID);
