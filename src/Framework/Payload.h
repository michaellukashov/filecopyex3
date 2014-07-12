#pragma once

#include "Node.h"

class Payload
{
public:
  Payload() {};
  virtual ~Payload() {};

  virtual void init(const String & AName);

  void addProperty(const String & AName, int def);
  void addProperty(const String & AName, float def);
  void addProperty(const String & AName, const String & def);
  Property & operator()(const String & AName) { return getProp(AName); }

  const String getName() const {return name; };

  void propSave() { propSaved = prop; };
  void propLoad() { prop = propSaved; };

protected:
  Property & getProp(const String & AName) { return prop[AName]; }

  PropertyMap prop;
  PropertyMap propSaved;
  String name;
};

#define DEFINE_CLASS(name, type) \
  static Payload* create() { return new type; };

//virtual const String getType() { return ; }

