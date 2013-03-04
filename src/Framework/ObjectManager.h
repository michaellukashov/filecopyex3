#ifndef __OBJECTMANAGER_H__
#define __OBJECTMANAGER_H__

#include "Array.h"
#include "ObjectClass.h"
#include "Object.h"

class ObjectManager
{
public:
	~ObjectManager();
	void RegClass(ObjectClass* cl);
	Object* Create(const String& type, const String& name, Object* parent);

private:
	ObjectClass* FindClass(const String& type);
	Array<ObjectClass*> reg_classes;
};

extern ObjectManager* objectManager;

#endif // __OBJECTMANAGER_H__