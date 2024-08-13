#ifndef POLSERVER_ARGUMENT_H
#define POLSERVER_ARGUMENT_H

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/model/ScopableName.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;

class Argument : public Node
{
public:
  Argument( const SourceLocation&, const ScopableName& identifier, std::unique_ptr<Expression>,
            bool spread );
  Argument( const SourceLocation&, std::unique_ptr<Expression>, bool spread );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  // can be empty if not specified
  const std::unique_ptr<ScopableName> identifier;

  const bool spread;

  std::unique_ptr<Expression> take_expression();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_ARGUMENT_H
