#ifndef POLSERVER_NODEVISITOR_H
#define POLSERVER_NODEVISITOR_H

namespace Pol::Bscript::Compiler
{
class Node;
class TopLevelStatements;
class ValueConsumer;

class NodeVisitor
{
public:
  virtual ~NodeVisitor() = default;

  virtual void visit_top_level_statements( TopLevelStatements& );
  virtual void visit_value_consumer( ValueConsumer& );

  virtual void visit_children( Node& parent );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_NODEVISITOR_H
