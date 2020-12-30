#ifndef POLSERVER_DOWHILELOOP_H
#define POLSERVER_DOWHILELOOP_H

#include "bscript/compiler/ast/LoopStatement.h"

namespace Pol::Bscript::Compiler
{
class Block;
class Expression;

class DoWhileLoop : public LoopStatement
{
public:
  DoWhileLoop( const SourceLocation& source_location, std::string label, std::unique_ptr<Block>,
               std::unique_ptr<Expression> );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  Block& block();
  Expression& predicate();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DOWHILELOOP_H
