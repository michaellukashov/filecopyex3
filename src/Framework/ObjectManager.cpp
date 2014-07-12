#include "ObjectManager.h"

ObjectManager * objectManager;

ObjectManager::~ObjectManager()
{
}


void ObjectManager::regClass(const String & type, createPayloadFunc pf, createNodeFunc nf)
{
  classes[type] = createFuncs(pf, nf);
}

Node * ObjectManager::create(const String & type, const String & name, Node * parent)
{
  createFuncs cf = classes[type];
  if(cf.nf && cf.pf)
  {
    Payload * payload = (*cf.pf)();
    payload->init(name);
    Node * node = (*cf.nf)();
    node->init(payload, parent);
    return node;
  }
  return NULL;
}

