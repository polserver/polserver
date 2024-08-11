#ifndef POLSERVER_VARSTATEMENT_H
#define POLSERVER_VARSTATEMENT_H

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;
class Variable;

class VarStatement : public Statement
{
public:
  // We have to keep track of scope in VarStatement, as class var statements are
  // "hoisted" to the top-level statements of the script.
  VarStatement( const SourceLocation&, std::string scope, std::string name,
                std::unique_ptr<Expression> initializer );
  VarStatement( const SourceLocation&, std::string scope, std::string name );
  VarStatement( const SourceLocation&, std::string scope, std::string name,
                bool initialize_as_empty_array );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const std::string scope;
  const std::string name;
  const bool initialize_as_empty_array = false;

  std::shared_ptr<Variable> variable;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_VARSTATEMENT_H
