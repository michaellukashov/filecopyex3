#include "ObjectManager.h"

ObjectManager *objectManager;

ObjectManager::~ObjectManager()
{
	for(int i = 0; i < reg_classes.Count(); ++i)
	{
		delete reg_classes[i];
	}
}
void ObjectManager::RegClass(ObjectClass* cl) 
{
	cl->DefineProperties();
	reg_classes.Add(cl);
}
Object* ObjectManager::Create(const String& type, const String& name, Object* parent)
{
	ObjectClass* cl = FindClass(type);
	if(cl)
	{
		Object* obj=cl->Create();
		obj->_class=cl;
		obj->_type=type;
		obj->_name=name;
		obj->_parent=parent;
		obj->properties = cl->Properties;
		if(parent)
			parent->childs.Add(obj);
		return obj;
	}
	return NULL;
}

ObjectClass* ObjectManager::FindClass(const String& type)
{
	for(int i = 0; i < reg_classes.Count(); ++i)
	{
		if(reg_classes[i]->TypeName() == type)
			return reg_classes[i];
	}
	return NULL;
}
