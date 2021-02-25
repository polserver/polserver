#ifndef POLSERVER_FOREACHLOOP_H
#define POLSERVER_FOREACHLOOP_H

#include "bscript/compiler/ast/LoopStatement.h"
#include "bscript/compiler/model/LocalVariableScopeInfo.h"

namespace Pol::Bscript::Compiler
{
class Block;
class Expression;
class Variable;

class ForeachLoop : public LoopStatement
{
public:
  ForeachLoop( const SourceLocation&, std::string label, std::string iterator_name,
               std::unique_ptr<Expression>, std::unique_ptr<Block> );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  Expression& expression();
  Block& block();

  const std::string iterator_name;
  LocalVariableScopeInfo local_variable_scope_info;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FOREACHLOOP_H
