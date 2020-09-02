#ifndef POLSERVER_BINARYOPERATOR_H
#define POLSERVER_BINARYOPERATOR_H

#include "compiler/ast/Expression.h"

#include "tokens.h"

namespace Pol::Bscript::Compiler
{
class BinaryOperator : public Expression
{
public:
  BinaryOperator( const SourceLocation&, std::unique_ptr<Expression> lhs, std::string op,
                  BTokenId, std::unique_ptr<Expression> rhs );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  Expression& lhs();
  Expression& rhs();

  std::unique_ptr<Expression> take_lhs();
  std::unique_ptr<Expression> take_rhs();

  const std::string op;
  const BTokenId token_id;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BINARYOPERATOR_H
