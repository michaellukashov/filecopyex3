#include "Common.h"
#include "FarSettings.h"
#include "guid.hpp"

#include "SDK/plugin.hpp"

FarSettings::FarSettings() :
  handle(INVALID_HANDLE_VALUE), dirId(0)
{
}

FarSettings::~FarSettings()
{
  clean();
}

intptr_t FarSettings::control(FAR_SETTINGS_CONTROL_COMMANDS cmd, void * param)
{
  return Info.SettingsControl(handle, cmd, 0, param);
}

void FarSettings::clean()
{
  if (handle != INVALID_HANDLE_VALUE)
  {
    control(SCTL_FREE);
    handle = INVALID_HANDLE_VALUE;
    dirId = 0;
  }
}

bool FarSettings::create()
{
  clean();
  FarSettingsCreate fsc = { sizeof(FarSettingsCreate) };
  fsc.Guid = MainGuid;
  if (!control(SCTL_CREATE, &fsc))
  {
    return false;
  }
  handle = fsc.Handle;
  FarSettingsValue fsv = { sizeof(FarSettingsValue) };
  fsv.Root = dirId;
  fsv.Value = L"FileCopyEx3Settings";
  intptr_t dir_id = control(SCTL_CREATESUBKEY, &fsv);
  if (dir_id == 0)
  {
    return false;
  }
  else
  {
    dirId = dir_id;
  }
  return true;
}

bool FarSettings::get(const String & name, String & value)
{
  FarSettingsItem fsi = { sizeof(FarSettingsItem) };
  fsi.Root = dirId;
  fsi.Name = name.c_str();
  fsi.Type = FST_STRING;
  if (!control(SCTL_GET, &fsi))
  {
    return false;
  }
  value = fsi.String;
  return true;
}

bool FarSettings::set(const String & name, const String & value)
{
  FarSettingsItem fsi = { sizeof(FarSettingsItem) };
  fsi.Root = dirId;
  fsi.Name = name.c_str();
  fsi.Type = FST_STRING;
  fsi.String = value.c_str();
  return control(SCTL_SET, &fsi) != 0;
}

bool FarSettings::list(ParamInfoVector & res)
{
  FarSettingsEnum fse = { sizeof(FarSettingsEnum) };
  fse.Root = dirId;
  if (!control(SCTL_ENUM, &fse))
  {
    return false;
  }
  res.clear();
  for (size_t Index = 0; Index < fse.Count; Index++)
  {
    if (fse.Items[Index].Type == FST_STRING)
    {
      res.push_back(ParamInfo(fse.Items[Index].Name, fse.Items[Index].Type));
    }
  }
  return true;
}

bool saveOptions(const PropertyMap & options, FarSettings & settings)
{
  bool ok = true;
  for (PropertyMap::const_iterator it = options.begin(); it != options.end(); ++it)
  {
    if (!settings.set(it->first, it->second.operator const String()))
    {
      ok = false;
    }
  }

  return ok;
}

bool loadOptions(PropertyMap & options, FarSettings & settings)
{
  FarSettings::ParamInfoVector v;

  if (!settings.list(v))
  {
    return false;
  }

  for (FarSettings::ParamInfoVector::iterator it = v.begin(); it != v.end(); ++it)
  {
    String & name = it->name;
    String v;
    if (settings.get(name, v))
    {
      options[name] = v;
    }
  }

  return true;
}

/*
bool Settings::set_dir(const wstring& path) {
  FarSettingsValue fsv = { sizeof(FarSettingsValue) };
  size_t dir_id = 0;
  list<wstring> dir_list = split(path, L'\\');
  for (list<wstring>::const_iterator dir = dir_list.cbegin(); dir != dir_list.cend(); dir++) {
    fsv.Root = dir_id;
    fsv.Value = dir->c_str();
    dir_id = control(SCTL_CREATESUBKEY, &fsv);
    if (dir_id == 0)
      return false;
  }
  this->dir_id = dir_id;
  return true;
}

bool Settings::list_dir(vector<wstring>& result) {
  FarSettingsEnum fse = { sizeof(FarSettingsEnum) };
  fse.Root = dir_id;
  if (!control(SCTL_ENUM, &fse))
    return false;
  result.clear();
  result.reserve(fse.Count);
  for (size_t i = 0; i < fse.Count; i++) {
    if (fse.Items[i].Type == FST_SUBKEY)
      result.push_back(fse.Items[i].Name);
  }
  result.shrink_to_fit();
  return true;
}

bool Settings::set(const wchar_t* name, uint64_t value) {
  FarSettingsItem fsi = { sizeof(FarSettingsItem) };
  fsi.Root = dir_id;
  fsi.Name = name;
  fsi.Type = FST_QWORD;
  fsi.Number = value;
  return control(SCTL_SET, &fsi) != 0;
}

bool Settings::set(const wchar_t* name, const wstring& value) {
  FarSettingsItem fsi = { sizeof(FarSettingsItem) };
  fsi.Root = dir_id;
  fsi.Name = name;
  fsi.Type = FST_STRING;
  fsi.String = value.c_str();
  return control(SCTL_SET, &fsi) != 0;
}

bool Settings::set(const wchar_t* name, const void* value, size_t value_size) {
  FarSettingsItem fsi = { sizeof(FarSettingsItem) };
  fsi.Root = dir_id;
  fsi.Name = name;
  fsi.Type = FST_DATA;
  fsi.Data.Data = value;
  fsi.Data.Size = value_size;
  return control(SCTL_SET, &fsi) != 0;
}

bool Settings::get(const wchar_t* name, uint64_t& value) {
  return get(dir_id, name, value);
}

bool Settings::get(const wchar_t* name, wstring& value) {
  return get(dir_id, name, value);
}

bool Settings::get(const wchar_t* name, ByteVector& value) {
  return get(dir_id, name, value);
}

bool Settings::get(size_t root, const wchar_t* name, uint64_t& value) {
  FarSettingsItem fsi = { sizeof(FarSettingsItem) };
  fsi.Root = root;
  fsi.Name = name;
  fsi.Type = FST_QWORD;
  if (!control(SCTL_GET, &fsi))
    return false;
  value = fsi.Number;
  return true;
}

bool Settings::get(size_t root, const wchar_t* name, wstring& value) {
  FarSettingsItem fsi = { sizeof(FarSettingsItem) };
  fsi.Root = root;
  fsi.Name = name;
  fsi.Type = FST_STRING;
  if (!control(SCTL_GET, &fsi))
    return false;
  value = fsi.String;
  return true;
}

bool Settings::get(size_t root, const wchar_t* name, ByteVector& value) {
  FarSettingsItem fsi = { sizeof(FarSettingsItem) };
  fsi.Root = root;
  fsi.Name = name;
  fsi.Type = FST_DATA;
  if (!control(SCTL_GET, &fsi))
    return false;
  const uint8_t* data = static_cast<const uint8_t*>(fsi.Data.Data);
  value.assign(data, data + fsi.Data.Size);
  return true;
}

bool Settings::del(const wchar_t* name) {
  FarSettingsValue fsv = { sizeof(FarSettingsValue) };
  fsv.Root = dir_id;
  fsv.Value = name;
  return control(SCTL_DELETE, &fsv) != 0;
}

bool Settings::del_dir(const wchar_t* name) {
  FarSettingsValue fsv = { sizeof(FarSettingsValue) };
  fsv.Root = dir_id;
  fsv.Value = name;
  size_t subdir_id = control(SCTL_OPENSUBKEY, &fsv);
  if (subdir_id == 0)
    return true;
  fsv.Root = subdir_id;
  fsv.Value = nullptr;
  return control(SCTL_DELETE, &fsv) != 0;
}

INT_PTR Settings::control(FAR_SETTINGS_CONTROL_COMMANDS command, void* param) {
  return Info.SettingsControl(handle, command, PSL_ROAMING, param);
}
*/
