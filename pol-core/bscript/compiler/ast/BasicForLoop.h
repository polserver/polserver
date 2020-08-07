#ifndef POLSERVER_BASICFORLOOP_H
#define POLSERVER_BASICFORLOOP_H

#include "LoopStatement.h"

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
                std::unique_ptr<Block> block );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  Expression& first();
  Expression& last();
  Block& block();

  const std::string identifier;
  std::vector<std::shared_ptr<Variable>> debug_variables;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BASICFORLOOP_H
