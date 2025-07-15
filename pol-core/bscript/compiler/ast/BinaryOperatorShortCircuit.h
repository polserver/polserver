#pragma once

#include "bscript/compiler/ast/Expression.h"

#include "tokens.h"

namespace Pol::Bscript::Compiler
{
class BinaryOperatorShortCircuit : public Expression
{
public:
  BinaryOperatorShortCircuit( const SourceLocation&, std::unique_ptr<Expression> lhs,
                              std::string op, BTokenId, std::unique_ptr<Expression> rhs );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  Expression& lhs();
  Expression& rhs();

  const std::string op;
  const BTokenId token_id;
};

}  // namespace Pol::Bscript::Compiler
