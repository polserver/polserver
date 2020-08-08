#ifndef POLSERVER_NODEVISITOR_H
#define POLSERVER_NODEVISITOR_H

namespace Pol::Bscript::Compiler
{
class Node;

class NodeVisitor
{
public:
  virtual ~NodeVisitor() = default;

  virtual void visit_children( Node& parent );
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_NODEVISITOR_H
