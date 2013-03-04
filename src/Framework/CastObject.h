#ifndef __CASTOBJECT_H__
#define __CASTOBJECT_H__

#include "Object.h"

template <class ChildType, class ParentType, class ClassType>
class CastObject : public Object
{
public:
	ClassType* Class() { return (ClassType*)_class; }
	ParentType* Parent() { return (ParentType*)_parent; }
	ChildType& operator[](int i) { return Child(i); }
	ChildType& operator[](const String& v) { return Child(v); }

protected:
	ChildType& Child(int i) { return static_cast<ChildType&>(Object::Child(i)); }
	ChildType& Child(const String& v) { return static_cast<ChildType&>(Object::Child(v)); }
};

#endif // __CASTOBJECT_H__