#ifndef POLSERVER_WHILELOOP_H
#define POLSERVER_WHILELOOP_H

#include "bscript/compiler/ast/LoopStatement.h"

namespace Pol::Bscript::Compiler
{
class Block;
class Expression;

class WhileLoop : public LoopStatement
{
public:
  WhileLoop( const SourceLocation& source_location, std::string label,
             std::unique_ptr<Expression> expression, std::unique_ptr<Block> block );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  Expression* predicate();
  Block& block();

  void remove_predicate();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_WHILELOOP_H
