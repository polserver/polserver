#ifndef POLSERVER_UNARYOPERATOR_H
#define POLSERVER_UNARYOPERATOR_H

#include "bscript/compiler/ast/Expression.h"

#ifndef __TOKENS_H
#include "tokens.h"
#endif

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class UnaryOperator : public Expression
{
public:
  UnaryOperator( const SourceLocation& source_location, std::string op, BTokenId token_id,
                 std::unique_ptr<Expression> operand );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;

  Expression& operand();
  std::unique_ptr<Expression> take_operand();

  const std::string op;
  const BTokenId token_id;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_UNARYOPERATOR_H
