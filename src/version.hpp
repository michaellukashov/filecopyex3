#pragma once

#include "sdk/farversion.hpp"

#define PLUGIN_MIN_FAR_VERSION MAKEFARVERSION(3, 0, 0, 2927, VS_RELEASE) // http://api.farmanager.com/ru/whatsnew.html
#define PLUGIN_VERSION_TXT "3.0.1.13"
#define PLUGIN_MAJOR 3
#define PLUGIN_MINOR 0
#define PLUGIN_SUBMINOR 1
#define PLUGIN_BUILD 13
#define PLUGIN_VERSION MAKEFARVERSION(PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_SUBMINOR, PLUGIN_BUILD, VS_BETA)
#define PLUGIN_DESC L"Extended File Copy plugin for Far 3 file manager main executable"
#define PLUGIN_NAME L"FileCopyEx"
#define PLUGIN_TITLE L"FileCopyEx"
#define PLUGIN_FILENAME L"FileCopyEx3.dll"
#define PLUGIN_AUTHOR L"Michael Lukashov, Ruslan Petrenko, djdron, craZZy, Max Antipin and others"
