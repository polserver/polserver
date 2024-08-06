#ifndef POLSERVER_ARGUMENT_H
#define POLSERVER_ARGUMENT_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;

class Argument : public Node
{
public:
  Argument( const SourceLocation&, std::string identifier, std::unique_ptr<Expression>,
            bool spread );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  const std::string identifier;  // can be empty if not specified
  const bool spread;

  std::unique_ptr<Expression> take_expression();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_ARGUMENT_H
