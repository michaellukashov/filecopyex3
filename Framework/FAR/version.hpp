#include "interface/farversion.hpp"

#define PLUGIN_BUILD 1
#define PLUGIN_DESC L"Extended File Copy plugin for Far 3 file manager main executable"
#define PLUGIN_NAME L"FileCopyEx"
#define PLUGIN_FILENAME L"FileCopyEx.dll"
#define PLUGIN_AUTHOR L"..., Ruslan Petrenko"
#define PLUGIN_VERSION MAKEFARVERSION(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,FARMANAGERVERSION_REVISION,PLUGIN_BUILD,VS_RELEASE)
