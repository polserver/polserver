#ifndef POLSERVER_TOPLEVELSTATEMENTS_H
#define POLSERVER_TOPLEVELSTATEMENTS_H

#include "Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Statement;

class TopLevelStatements : public Node
{
public:
  TopLevelStatements( const SourceLocation&, std::vector<std::unique_ptr<Statement>> );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_TOPLEVELSTATEMENTS_H
