#include "Node.h"

template <class ChildType, class ParentType, class PayloadType>
class CastNode : public Node
{
public:
  ParentType * Parent() { return static_cast<ParentType *>(parent); }
  ChildType & operator[](int i) { return child(i); }
  ChildType & operator[](const String & name) { return child(name); }
  PayloadType & getPayload() const { return static_cast<PayloadType &>(Node::getPayload()); }

protected:
  ChildType & child(size_t i) { return static_cast<ChildType &>(Node::child(i)); }
  ChildType & child(const String & name) { return static_cast<ChildType &>(Node::child(name)); }
};
