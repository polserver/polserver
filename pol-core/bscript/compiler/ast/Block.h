#ifndef POLSERVER_BLOCK_H
#define POLSERVER_BLOCK_H

#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/model/LocalVariableScopeInfo.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Statement;
class Variable;

class Block : public Statement
{
public:
  Block( const SourceLocation&, std::vector<std::unique_ptr<Statement>> statements );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  // set by semantic analyzer:
  LocalVariableScopeInfo local_variable_scope_info;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BLOCK_H
