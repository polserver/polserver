#ifndef POLSERVER_REPEATUNTILLOOP_H
#define POLSERVER_REPEATUNTILLOOP_H

#include "compiler/ast/LoopStatement.h"

namespace Pol::Bscript::Compiler
{
class Block;
class Expression;
class FlowControlLabel;

class RepeatUntilLoop : public LoopStatement
{
public:
  RepeatUntilLoop( const SourceLocation&, std::string label, std::unique_ptr<Block>,
                   std::unique_ptr<Expression> );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  Block& block();
  Expression& expression();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_REPEATUNTILLOOP_H
