#pragma once

#include <stddef.h>
#include "SDK/plugin.hpp"
#include "Framework/Properties.h"

class FarSettings
{
public:
  FarSettings();
  ~FarSettings();

  bool create();
  bool get(const String & name, String & value);
  bool set(const String & name, const String & value);

  class ParamInfo
  {
  public:
    String name;
    FARSETTINGSTYPES type;

    ParamInfo(const String & _name, FARSETTINGSTYPES _type) :
      name(_name),
      type(_type)
    {}
  };
  typedef std::vector<ParamInfo> ParamInfoVector;

  bool list(ParamInfoVector & res);
private:
  HANDLE handle;
  size_t dirId;

  intptr_t control(FAR_SETTINGS_CONTROL_COMMANDS cmd, void * param = nullptr);
  void clean();
};

bool saveOptions(const PropertyMap & options, FarSettings & settings);
bool loadOptions(PropertyMap & options, FarSettings & settings);
