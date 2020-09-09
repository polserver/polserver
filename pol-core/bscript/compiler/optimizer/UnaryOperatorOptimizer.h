#ifndef POLSERVER_UNARYOPERATOROPTIMIZER_H
#define POLSERVER_UNARYOPERATOROPTIMIZER_H

#include "compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class Expression;
class Identifier;
class IntegerValue;
class FloatValue;
class UnaryOperator;

class UnaryOperatorOptimizer : public NodeVisitor
{
public:
  explicit UnaryOperatorOptimizer( UnaryOperator& unary_operator );

  std::unique_ptr<Expression> optimize();

  void visit_children( Node& ) override;
  void visit_float_value( FloatValue& literal ) override;
  void visit_get_member( GetMember& ) override;
  void visit_integer_value( IntegerValue& literal ) override;

private:
  std::unique_ptr<Expression> optimized_result;

  UnaryOperator& unary_operator;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_UNARYOPERATOROPTIMIZER_H
