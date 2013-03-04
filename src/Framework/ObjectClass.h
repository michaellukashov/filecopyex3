#ifndef __OBJECTCLASS_H__
#define __OBJECTCLASS_H__

#include "Object.h"

class Object;

class ObjectClass
{
public:
	virtual Object* Create() { return NULL; }
	virtual const String TypeName() { return String(); }
	virtual void DefineProperties() {}

	void AddProperty(const String& name, int def);
	void AddProperty(const String& name, float def);
	void AddProperty(const String& name, const String& def);

	Property& operator()(const String& v) { return Property(v); }

	Property& Property(const String& v);

private:
	StringList PropertyNames;
	PropertyStore Properties;

	friend class ObjectManager;
	friend class Object;
};

#endif // __OBJECTCLASS_H__