#ifndef POLSERVER_BINARYOPERATORWITHFLOATOPTIMIZER_H
#define POLSERVER_BINARYOPERATORWITHFLOATOPTIMIZER_H

#include "compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class BinaryOperator;
class Expression;
class FloatValue;
class Report;

class BinaryOperatorWithFloatOptimizer : public NodeVisitor
{
public:
  const FloatValue& lhs;
  const BinaryOperator& op;
  Report& report;

  std::unique_ptr<Expression> optimized_result;

  BinaryOperatorWithFloatOptimizer( FloatValue& lhs, BinaryOperator& op, Report& );

  void visit_children( Node& parent ) override;
  void visit_float_value( FloatValue& rhs ) override;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_BINARYOPERATORWITHFLOATOPTIMIZER_H
