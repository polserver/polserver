#ifndef POLSERVER_BASICFORLOOP_H
#define POLSERVER_BASICFORLOOP_H

#include "compiler/ast/LoopStatement.h"
#include "compiler/model/LocalVariableScopeInfo.h"

namespace Pol::Bscript::Compiler
{
class Block;
class Expression;
class Variable;

class BasicForLoop : public LoopStatement
{
public:
  BasicForLoop( const SourceLocation&, std::string label, std::string identifier,
                std::unique_ptr<Expression> first, std::unique_ptr<Expression> last,
                std::unique_ptr<Block> );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  Expression& first();
  Expression& last();
  Block& block();

  const std::string identifier;
  LocalVariableScopeInfo local_variable_scope_info;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BASICFORLOOP_H
