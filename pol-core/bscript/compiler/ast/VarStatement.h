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
  VarStatement( const SourceLocation& identifier_location, const SourceLocation& var_decl_location,
                std::string name, std::unique_ptr<Expression> initializer );
  VarStatement( const SourceLocation& identifier_location, const SourceLocation& var_decl_location,
                std::string name );
  VarStatement( const SourceLocation& identifier_location, const SourceLocation& var_decl_location,
                std::string name, bool initialize_as_empty_array );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;

  const std::string name;
  const bool initialize_as_empty_array = false;

  std::shared_ptr<Variable> variable;
  const SourceLocation var_decl_location;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_VARSTATEMENT_H
