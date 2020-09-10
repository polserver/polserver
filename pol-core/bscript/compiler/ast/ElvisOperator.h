#ifndef POLSERVER_ELVISOPERATOR_H
#define POLSERVER_ELVISOPERATOR_H

#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class ElvisOperator : public Expression
{
public:
  ElvisOperator( const SourceLocation&, std::unique_ptr<Expression> lhs,
                 std::unique_ptr<Expression> rhs );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  Expression& lhs();
  Expression& rhs();
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_ELVISOPERATOR_H
