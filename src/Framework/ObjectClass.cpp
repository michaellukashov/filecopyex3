#include "ObjectClass.h"

void ObjectClass::AddProperty(const String& name, int def)
{
	PropertyNames.Add(name, Properties.Add(def));
}

void ObjectClass::AddProperty(const String& name, float def)
{
	PropertyNames.Add(name, Properties.Add(def));
}

void ObjectClass::AddProperty(const String& name, const String& def)
{
	PropertyNames.Add(name, Properties.Add(def));
}
