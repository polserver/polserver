#ifndef POLSERVER_IDENTIFIER_H
#define POLSERVER_IDENTIFIER_H

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class FunctionLink;
class Variable;

class Identifier : public Expression
{
public:
  Identifier( const SourceLocation&, std::string calling_scope, std::string name );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string maybe_scoped_name() const;

  // Used in SemanticAnalyzer. If the identifer does not exist under `name`,
  // check `calling_scope::name` (if calling scope exists).
  const std::string calling_scope;
  const std::string name;

  std::shared_ptr<Variable> variable;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_IDENTIFIER_H
