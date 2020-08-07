#ifndef POLSERVER_BLOCK_H
#define POLSERVER_BLOCK_H

#include "Statement.h"

namespace Pol::Bscript::Compiler
{
class DebugBlock;
class NodeVisitor;
class Statement;
class Variable;

class Block : public Statement
{
public:
  Block( const SourceLocation&, std::vector<std::unique_ptr<Statement>> statements );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  // set by semantic analyzer:
  unsigned locals_in_block;
  std::vector<std::shared_ptr<Variable>> debug_variables;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BLOCK_H
