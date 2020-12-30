#ifndef POLSERVER_STATEMENT_H
#define POLSERVER_STATEMENT_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Statement : public Node
{
public:
  explicit Statement( const SourceLocation& );
  Statement( const SourceLocation&, std::unique_ptr<Node> childr );
  Statement( const SourceLocation&, NodeVector children );

  template <typename T>
  Statement( const SourceLocation& source_location, std::vector<std::unique_ptr<T>> children )
      : Node( source_location, std::move( children ) )
  {
  }
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_STATEMENT_H
