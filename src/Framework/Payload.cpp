#include "Payload.h"

void Payload::addProperty(const String & AName, int def)
{
  prop[AName] = def;
}

void Payload::addProperty(const String & AName, float def)
{
  prop[AName] = def;
}

void Payload::addProperty(const String & AName, const String & def)
{
  prop[AName] = def;
}

void Payload::init(const String & AName)
{
  name = AName;
}
