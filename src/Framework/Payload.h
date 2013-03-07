#ifndef __OBJECTCLASS_H__
#define __OBJECTCLASS_H__

#include "Node.h"

class Payload
{
public:
	Payload() {};
	~Payload() {};

	virtual void init(const String &_name);

	void addProperty(const String& name, int def);
	void addProperty(const String& name, float def);
	void addProperty(const String& name, const String& def);
	Property& operator()(const String& name) { return getProp(name); }

	const String getName() const {return name; };

	void propSave() { propSaved = prop; };
	void propLoad() { prop = propSaved; };

protected:
	Property& getProp(const String& name) { return prop[name]; }

	PropertyMap prop;
	PropertyMap propSaved;
	String name;
};

#define DEFINE_CLASS(name, type) \
	static Payload* create() { return new type; };
	
//virtual const String getType() { return ; }

#endif // __OBJECTCLASS_H__