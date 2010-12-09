#pragma once

#ifdef _WINXP
#define _WIN2K
#endif

#ifdef _WIN2K
#define _WINNT
#endif

#if defined(_WINXP)
#define _WIN32_WINNT 0x0501
#elif defined(_WIN2K)
#define _WIN32_WINNT 0x0500
#elif defined(_WINNT)
#define _WIN32_WINNT 0x0400
#endif

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <tchar.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

#include <windows.h>
#include <winioctl.h>

//#pragma hdrstop