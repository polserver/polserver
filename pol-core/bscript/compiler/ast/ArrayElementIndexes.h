#ifndef POLSERVER_ARRAYELEMENTINDEXES_H
#define POLSERVER_ARRAYELEMENTINDEXES_H

#include "Node.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class ArrayElementIndexes : public Node
{
public:
  ArrayElementIndexes( const SourceLocation&, std::vector<std::unique_ptr<Expression>> );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_ARRAYELEMENTINDEXES_H
