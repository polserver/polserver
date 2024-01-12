#ifndef POLSERVER_UNARYOPERATOROPTIMIZER_H
#define POLSERVER_UNARYOPERATOROPTIMIZER_H

#include "bscript/compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class BooleanValue;
class Expression;
class Identifier;
class IntegerValue;
class FloatValue;
class UnaryOperator;
class UninitializedValue;

class UnaryOperatorOptimizer : public NodeVisitor
{
public:
  explicit UnaryOperatorOptimizer( UnaryOperator& unary_operator );

  std::unique_ptr<Expression> optimize();

  void visit_children( Node& ) override;
  void visit_boolean_value( BooleanValue& ) override;
  void visit_uninitialized_value( UninitializedValue& ) override;
  void visit_float_value( FloatValue& literal ) override;
  void visit_integer_value( IntegerValue& literal ) override;
  void visit_member_access( MemberAccess& ) override;

private:
  std::unique_ptr<Expression> optimized_result;

  UnaryOperator& unary_operator;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_UNARYOPERATOROPTIMIZER_H
