#ifndef POLSERVER_ELEMENTINDEXES_H
#define POLSERVER_ELEMENTINDEXES_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class ElementIndexes : public Node
{
public:
  ElementIndexes( const SourceLocation&, std::vector<std::unique_ptr<Expression>> );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_ELEMENTINDEXES_H
