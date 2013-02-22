#include "common.h"

#include <initguid.h>
#include "guid.hpp"

#include "SDK/plugin.hpp"
#include "FarPlugin.h"
#include "version.hpp"
#include "dlgclass.h"

PluginStartupInfo Info;
FarPlugin* plugin = NULL;

/*
 Функция GetMsg возвращает строку сообщения из языкового файла.
 А это надстройка над Info.GetMsg для сокращения кода :-)
*/
const wchar_t* GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

static void FarErrorHandler(const wchar_t* s)
{
	/*if (ShowMessageEx("Error", s, "OK\nDebug", 0)==1)
	DebugBreak();*/
	const wchar_t* items[]={ L"Framework Error", s, L"OK", L"Debug" };
	if (Info.Message(&MainGuid, &MainGuid, FMSG_WARNING, NULL, items, 4, 2)==1)
		DebugBreak();
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
	Info = *psi;
	if(!plugin)
	{
		errorHandler = FarErrorHandler;
		InitObjMgr();
		plugin = CreatePlugin();
	}
}

/*
  Функция OpenW вызывается при создании новой копии плагина.
*/
HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	plugin->InitLang();
	plugin->OpenPlugin(OInfo);
	plugin->SaveOptions();
	//  if (p) return (HANDLE)p;
	//  else
	return INVALID_HANDLE_VALUE;
}

int WINAPI ConfigureW(int ItemNumber)
{
	plugin->InitLang();
	int res=plugin->Configure(ItemNumber);
	plugin->SaveOptions();
	return res;
}

/*
Функция GetPluginInfoW вызывается для получения информации о плагине
*/
void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize = sizeof(*Info);
	Info->Flags = PF_EDITOR;
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0] = L"FileCopyEx"; //GetMsg(MTitle);
	Info->PluginMenu.Guids = &MenuGuid;
	Info->PluginMenu.Strings = PluginMenuStrings;
	Info->PluginMenu.Count = ARRAYSIZE(PluginMenuStrings);

	plugin->InitLang();
	plugin->FillInfo(Info);
}

void WINAPI ClosePluginW(HANDLE hPlugin)
{
	plugin->SaveOptions();
	//  delete (FarPanel*)hPlugin;
}

void WINAPI ExitFARW()
{
	delete plugin;
	DoneObjMgr();
};

const String& LOC(const String& l)
{
	return plugin->Locale()[l];
}

int WINAPI GetMinFarVersionW(void)
{
	return 0; // XXX FARMANAGERVERSION;
}