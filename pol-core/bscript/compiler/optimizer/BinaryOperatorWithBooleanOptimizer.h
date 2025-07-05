#ifndef POLSERVER_BINARYOPERATORWITHBOOLEANOPTIMIZER_H
#define POLSERVER_BINARYOPERATORWITHBOOLEANOPTIMIZER_H

#include "bscript/compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class Expression;
class Report;

class BinaryOperatorWithBooleanOptimizer : public NodeVisitor
{
public:
  const BooleanValue& lhs;
  const BinaryOperator& op;
  Report& report;

  std::unique_ptr<Expression> optimized_result;

  BinaryOperatorWithBooleanOptimizer( BooleanValue& lhs, BinaryOperator& op, Report& );

  void visit_children( Node& parent ) override;
  void visit_boolean_value( BooleanValue& rhs ) override;
  void visit_integer_value( IntegerValue& rhs ) override;
  void visit_float_value( FloatValue& rhs ) override;
  void visit_string_value( StringValue& rhs ) override;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_BINARYOPERATORWITHBOOLEANOPTIMIZER_H
