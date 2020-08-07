#ifndef POLSERVER_VARSTATEMENT_H
#define POLSERVER_VARSTATEMENT_H

#include "Statement.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;
class Variable;

class VarStatement : public Statement
{
public:
  VarStatement( const SourceLocation& source_location, std::string name,
                std::unique_ptr<Expression> initializer );
  VarStatement( const SourceLocation&, std::string name );
  VarStatement( const SourceLocation&, std::string name, bool initialize_as_empty_array );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  const std::string name;
  const bool initialize_as_empty_array = false;

  std::shared_ptr<Variable> variable;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_VARSTATEMENT_H
