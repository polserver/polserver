#ifndef POLSERVER_IDENTIFIER_H
#define POLSERVER_IDENTIFIER_H

#include "Expression.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class ConstDeclaration;
class FunctionLink;
class Variable;

class Identifier : public Expression
{
public:
  Identifier( const SourceLocation&, std::string name );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  std::string name;
  std::shared_ptr<Variable> variable;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_IDENTIFIER_H
