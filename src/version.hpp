#include "sdk/farversion.hpp"

#define PLUGIN_MIN_FAR_VERSION MAKEFARVERSION(3, 0, 0, 2927, VS_RELEASE) // http://api.farmanager.com/ru/whatsnew.html
#define PLUGIN_BUILD 8
#define PLUGIN_VERSION MAKEFARVERSION(FARMANAGERVERSION_MAJOR, FARMANAGERVERSION_MINOR, 2927, PLUGIN_BUILD, VS_BETA)
#define PLUGIN_DESC L"Extended File Copy plugin for Far 3 file manager main executable"
#define PLUGIN_TITLE L"FileCopyEx"
// #define PLUGIN_FILENAME L"FileCopyEx.dll"
#define PLUGIN_AUTHOR L"Ruslan Petrenko, djdron, craZZy, Max Antipin and other"
