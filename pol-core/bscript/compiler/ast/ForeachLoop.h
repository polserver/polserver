#ifndef POLSERVER_FOREACHLOOP_H
#define POLSERVER_FOREACHLOOP_H

#include "compiler/ast/LoopStatement.h"

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
  std::vector<std::shared_ptr<Variable>> debug_variables;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FOREACHLOOP_H
