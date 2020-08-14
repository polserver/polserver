#ifndef POLSERVER_TOPLEVELSTATEMENTS_H
#define POLSERVER_TOPLEVELSTATEMENTS_H

#include "compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Statement;

class TopLevelStatements : public Node
{
public:
  explicit TopLevelStatements( const SourceLocation& );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_TOPLEVELSTATEMENTS_H
