#include "common.h"

#include <initguid.h>
#include "guid.hpp"

#include "SDK/plugin.hpp"
#include "FarPlugin.h"
#include "version.hpp"
#include "FarPayload.h"

PluginStartupInfo Info;
FarStandardFunctions FSF;

FarPlugin * plugin = NULL;

/*
 Функция GetMsg возвращает строку сообщения из языкового файла.
 А это надстройка над Info.GetMsg для сокращения кода :-)
*/
const wchar_t * GetMsg(int MsgId)
{
  return Info.GetMsg(&MainGuid,MsgId);
}

static void FarErrorHandler(const wchar_t * s)
{
  const wchar_t * items[]= { L"Framework Error", s, L"OK", L"Debug" };
  if(Info.Message(&MainGuid, &MainGuid, FMSG_WARNING, NULL, items, 4, 2) == 1)
  {
    DebugBreak();
  }
}

/*
Far Manager вызывает функцию GetGlobalInfoW в первую очередь, для получения основной информации о плагине.
Функция вызывается один раз.
*/
void WINAPI GetGlobalInfoW(struct GlobalInfo * Info)
{
  Info->StructSize = sizeof(GlobalInfo);
  Info->MinFarVersion = PLUGIN_MIN_FAR_VERSION;
  Info->Version = PLUGIN_VERSION;
  Info->Guid = MainGuid;
  Info->Title = PLUGIN_TITLE;
  Info->Description = PLUGIN_DESC;
  Info->Author = PLUGIN_AUTHOR;
}

/*
Функция SetStartupInfoW вызывается один раз, после загрузки DLL-модуля в память.
Far Manager передаёт плагину информацию, необходимую для дальнейшей работы.
*/
void WINAPI SetStartupInfoW(const struct PluginStartupInfo * psi)
{
  if(psi->StructSize >= sizeof(PluginStartupInfo))
  {
    Info = *psi;
    FSF = *Info.FSF;
    if(!plugin)
    {
      errorHandler = FarErrorHandler;
      InitObjMgr();
      plugin = new FarPlugin();
      plugin->Create();
    }
  }
};

/*
Функция GetPluginInfoW вызывается Far Manager для получения дополнительной информации о плагине.
*/
static const wchar_t * pluginMenuStrings[1] = { L"Extended copy" };
static const wchar_t * configMenuStrings[1] = { L"Extended copy" };

void WINAPI GetPluginInfoW(struct PluginInfo * Info)
{
  Info->StructSize = sizeof(*Info);

  Info->PluginMenu.Guids = &MenuGuid;
  Info->PluginMenu.Strings = pluginMenuStrings;
  Info->PluginMenu.Count = ARRAYSIZE(pluginMenuStrings);

  Info->PluginConfig.Guids = &ConfigGuid;
  Info->PluginConfig.Strings = configMenuStrings;
  Info->PluginConfig.Count = ARRAYSIZE(configMenuStrings);

  plugin->InitLang();
}

/*
Функция ConfigureW вызывается Far Manager, когда пользователь выбрал в меню "Параметры внешних модулей" пункт, добавленный туда данным плагином.
*/
intptr_t WINAPI ConfigureW(const struct ConfigureInfo * Info)
{
  return plugin->Configure(Info);
}

/*
  Функция OpenW вызывается при создании новой копии плагина.
*/
HANDLE WINAPI OpenW(const struct OpenInfo * OInfo)
{
  plugin->InitLang();
  plugin->OpenPlugin(OInfo);
  plugin->SaveOptions();
  return 0; // INVALID_HANDLE_VALUE; // ok
}

void WINAPI ExitFARW(const struct ExitInfo * Info)
{
  delete plugin;
  DoneObjMgr();
};
