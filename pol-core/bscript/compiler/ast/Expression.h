#ifndef POLSERVER_EXPRESSION_H
#define POLSERVER_EXPRESSION_H

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class Expression : public Statement
{
public:
  explicit Expression( const SourceLocation& );

  Expression( const SourceLocation&, std::unique_ptr<Node> child );
  Expression( const SourceLocation&, NodeVector children );

  template <typename T>
  Expression( const SourceLocation& source_location, std::vector<std::unique_ptr<T>> children )
      : Statement( source_location, std::move( children ) )
  {
  }
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_EXPRESSION_H
