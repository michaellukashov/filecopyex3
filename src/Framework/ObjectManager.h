#ifndef __OBJECTMANAGER_H__
#define __OBJECTMANAGER_H__

//#include "Array.h"
#include "Payload.h"
#include "Node.h"

class ObjectManager
{
public:
	~ObjectManager();

	typedef Payload* createFunc();

	void regClass(const String& type, createFunc f);
	Node* create(const String& type, const String& name, Node* parent);
	
private:
	std::map<String, createFunc*> classes;
};

extern ObjectManager *objectManager;

#endif // __OBJECTMANAGER_H__