#ifndef POLSERVER_CONDITIONALOPERATOR_H
#define POLSERVER_CONDITIONALOPERATOR_H

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class FlowControlLabel;
class ConditionalOperator : public Expression
{
public:
  ConditionalOperator( const SourceLocation&, std::unique_ptr<Expression> conditional,
                       std::unique_ptr<Expression> consequent,
                       std::unique_ptr<Expression> alternate );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  Expression& conditional();
  Expression& consequent();
  Expression& alternate();
  const std::shared_ptr<FlowControlLabel> consequent_label;
  const std::shared_ptr<FlowControlLabel> alternate_label;

  std::unique_ptr<Expression> take_consequent();
  std::unique_ptr<Expression> take_alternate();
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_CONDITIONALOPERATOR_H
