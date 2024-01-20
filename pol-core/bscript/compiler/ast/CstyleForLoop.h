#ifndef POLSERVER_CSTYLEFORLOOP_H
#define POLSERVER_CSTYLEFORLOOP_H

#include "bscript/compiler/ast/LoopStatement.h"

namespace Pol::Bscript::Compiler
{
class Block;
class Expression;

class CstyleForLoop : public LoopStatement
{
public:
  CstyleForLoop( const SourceLocation&, std::string label, std::unique_ptr<Expression> initializer,
                 std::unique_ptr<Expression> predicate, std::unique_ptr<Expression> advancer,
                 std::unique_ptr<Block> );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  Expression& initializer();
  Expression& predicate();
  Expression& advancer();
  Block& block();
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_CSTYLEFORLOOP_H
