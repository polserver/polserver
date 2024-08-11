#ifndef POLSERVER_IDENTIFIER_H
#define POLSERVER_IDENTIFIER_H

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/model/ScopableName.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class FunctionLink;
class Variable;

class Identifier : public Expression
{
public:
  Identifier( const SourceLocation&, const ScopableName& scoped_name );
  Identifier( const SourceLocation&, const std::string& name );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  // Returns foo, Animal::foo but never ::foo
  std::string string() const;

  // Returns foo, never Animal::foo or ::foo
  const std::string& name() const;

  // Used in SemanticAnalyzer to differentiate between "foo" and "::foo".
  const ScopableName scoped_name;
  std::shared_ptr<Variable> variable;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_IDENTIFIER_H
