#ifndef POLSERVER_NODEVISITOR_H
#define POLSERVER_NODEVISITOR_H

namespace Pol::Bscript::Compiler
{
class NodeVisitor
{
public:
  virtual ~NodeVisitor() = default;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_NODEVISITOR_H
