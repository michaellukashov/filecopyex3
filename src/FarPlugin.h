/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010
Idea & core: Max Antipin
Coding: Serge Cheperis aka craZZy
Bugfixes: slst, CDK, Ivanych, Alter, Axxie and Nsky
Special thanks to Vitaliy Tsubin
Far 2 (32 & 64 bit) full unicode version by djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Framework/ObjString.h"
#include "Framework/LocaleUtils.h"
#include "Framework/Properties.h"
#include "Framework/StringVector.h"
#include "FarNode.h"
#include "FarSettings.h"

class FarPlugin
{
public:
  FarPlugin() : flags(0) {}
  ~FarPlugin();
  void Create();
  intptr_t Configure(const struct ConfigureInfo * Info);
  void OpenPlugin(const struct OpenInfo * OInfo);
  void InitLang();
  void LoadOptions();
  void SaveOptions();
  void InitOptions();
  void Config();

  const Locale & getLocale() const { return locale; }
  PropertyMap & Options() { return options; }
  FarDialogList & Dialogs() { return dialogs; }
  FarSettings & Settings() { return settings; }

  const StringVector & Descs() const { return descs; }

private:
  static String GetDLLPath();
  void UpdateConfiguration();

  FarDialogList dialogs;
  PropertyMap options;
  FarSettings settings;
  String CurLocaleFile;
  Locale locale;
  StringVector descs;

  int flags;

  void About();
  static void KeyConfig();
  void SoundConfig();
};
