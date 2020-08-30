#ifndef POLSERVER_FUNCTIONBODY_H
#define POLSERVER_FUNCTIONBODY_H

#include "compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Statement;

class FunctionBody : public Node // This is used for both UserFunction and Program.  Better name?
{
public:
  FunctionBody( const SourceLocation&, std::vector<std::unique_ptr<Statement>> );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  Statement* last_statement();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FUNCTIONBODY_H
