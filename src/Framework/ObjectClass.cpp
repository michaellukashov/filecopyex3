#include "ObjectClass.h"

void ObjectClass::AddProperty(const String& name, int def)
{
	baseProp[name] = def;
}

void ObjectClass::AddProperty(const String& name, float def)
{
	baseProp[name] = def;
}

void ObjectClass::AddProperty(const String& name, const String& def)
{
	baseProp[name] = def;
}
