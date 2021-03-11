#ifndef POLSERVER_CONDITIONALEXPRESSION_H
#define POLSERVER_CONDITIONALEXPRESSION_H

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class FlowControlLabel;
class ConditionalExpression : public Expression
{
public:
  ConditionalExpression( const SourceLocation&, std::unique_ptr<Expression> conditional,
                         std::unique_ptr<Expression> consequent,
                         std::unique_ptr<Expression> alternate );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  Expression& conditional();
  Expression& consequent();
  Expression& alternate();
  const std::shared_ptr<FlowControlLabel> consequent_label;
  const std::shared_ptr<FlowControlLabel> alternate_label;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_CONDITIONALEXPRESSION_H
