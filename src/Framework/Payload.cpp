#include "Payload.h"

void Payload::addProperty(const String& name, int def)
{
	prop[name] = def;
}

void Payload::addProperty(const String& name, float def)
{
	prop[name] = def;
}

void Payload::addProperty(const String& name, const String& def)
{
	prop[name] = def;
}

void Payload::init(const String &_name)
{
	name = _name;
}
