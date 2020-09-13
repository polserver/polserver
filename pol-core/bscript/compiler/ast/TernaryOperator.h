#ifndef POLSERVER_TERNARYOPERATOR_H
#define POLSERVER_TERNARYOPERATOR_H

#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class TernaryOperator : public Expression
{
public:
  TernaryOperator( const SourceLocation&, std::unique_ptr<Expression> predicate,
                   std::unique_ptr<Expression> consequent,
                   std::unique_ptr<Expression> alternative );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  Expression& predicate();
  Expression& consequent();
  Expression& alternative();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_TERNARYOPERATOR_H
