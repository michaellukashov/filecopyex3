#include "ObjectManager.h"

ObjectManager *objectManager;

ObjectManager::~ObjectManager()
{
}


void ObjectManager::regClass(const String& type, createFunc f)
{
	classes[type] = f;
}

Node* ObjectManager::create(const String& type, const String& name, Node* parent)
{
	createFunc *clCreate = classes[type];
	if (clCreate) {
		Payload *payload = (*clCreate)();
		payload->init(name);
		Node* node = new Node(payload, parent);
		return node;
	}
	return NULL;
}

