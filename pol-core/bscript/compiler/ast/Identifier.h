#ifndef POLSERVER_IDENTIFIER_H
#define POLSERVER_IDENTIFIER_H

#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class FunctionLink;
class Variable;

class Identifier : public Expression
{
public:
  Identifier( const SourceLocation&, std::string name );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  const std::string name;

  std::shared_ptr<Variable> variable;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_IDENTIFIER_H
