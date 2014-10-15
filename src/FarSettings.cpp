#include "Common.h"
#include "FarSettings.h"
#include "guid.hpp"

#include "SDK/plugin.hpp"

FarSettings::FarSettings() :
  handle(INVALID_HANDLE_VALUE), dirId(0), nAttachments(0)
{
}

FarSettings::~FarSettings()
{
  while (nAttachments != 0)
    detach();
}

intptr_t FarSettings::control(FAR_SETTINGS_CONTROL_COMMANDS cmd, void * param)
{
  return Info.SettingsControl(handle, cmd, 0, param);
}

void FarSettings::detach()
{
  if (--nAttachments == 0)
  {
    if (handle != INVALID_HANDLE_VALUE)
    {
      control(SCTL_FREE);
      handle = INVALID_HANDLE_VALUE;
      dirId = 0;
    }
  }
}

bool FarSettings::attach()
{
  if (nAttachments++ == 0)
  {
    FarSettingsCreate fsc = { sizeof(FarSettingsCreate) };
    fsc.Guid = MainGuid;
    if (!control(SCTL_CREATE, &fsc))
    {
      nAttachments--;
      return false;
    }
    handle = fsc.Handle;
    FarSettingsValue fsv = { sizeof(FarSettingsValue) };
    fsv.Root = dirId;
    fsv.Value = L"FileCopyEx3Settings";
    intptr_t dir_id = control(SCTL_CREATESUBKEY, &fsv);
    if (dir_id != 0)
    {
      dirId = dir_id;
    }
  }
  return true;
}

bool FarSettings::get(const String & name, String & value)
{
  if (!attach())
    return false;
  FarSettingsItem fsi = { sizeof(FarSettingsItem) };
  fsi.Root = dirId;
  fsi.Name = name.c_str();
  fsi.Type = FST_STRING;
  bool ok = control(SCTL_GET, &fsi) != 0;
  if (ok)
    value = fsi.String;
  detach();
  return ok;
}

bool FarSettings::set(const String & name, const String & value)
{
  if (!attach())
    return false;
  FarSettingsItem fsi = { sizeof(FarSettingsItem) };
  fsi.Root = dirId;
  fsi.Name = name.c_str();
  fsi.Type = FST_STRING;
  fsi.String = value.c_str();
  bool ok = control(SCTL_SET, &fsi) != 0;
  detach();
  return ok;
}

bool FarSettings::list(ParamInfoVector & res)
{
  if (!attach())
    return false;
  FarSettingsEnum fse = { sizeof(FarSettingsEnum) };
  fse.Root = dirId;
  bool ok = control(SCTL_ENUM, &fse) != 0;
  if (ok)
  {
    res.clear();
    for (size_t Index = 0; Index < fse.Count; Index++)
    {
      if (fse.Items[Index].Type == FST_STRING)
      {
        res.push_back(ParamInfo(fse.Items[Index].Name, fse.Items[Index].Type));
      }
    }
  }
  detach();
  return ok;
}

bool saveOptions(const PropertyMap & options, FarSettings & settings)
{
  bool ok = settings.attach();
  if (ok)
  {
    for (PropertyMap::const_iterator it = options.begin(); it != options.end(); ++it)
    {
      if (!settings.set(it->first, it->second.operator const String()))
      {
        ok = false;
      }
    }
    settings.detach();
  }
  return ok;
}

bool loadOptions(PropertyMap & options, FarSettings & settings)
{
  bool ok = settings.attach();
  if (ok)
  {
    FarSettings::ParamInfoVector v;
    ok = settings.list(v);
    if (ok)
    {
      for (FarSettings::ParamInfoVector::iterator it = v.begin(); it != v.end(); ++it)
      {
        String & name = it->name;
        String v;
        if (settings.get(name, v))
        {
          options[name] = v;
        }
      }
    }
    settings.detach();
  }
  return ok;
}
