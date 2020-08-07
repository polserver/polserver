#ifndef POLSERVER_BINARYOPERATOROPTIMIZER_H
#define POLSERVER_BINARYOPERATOROPTIMIZER_H

#include "compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class BinaryOperator;
class Expression;
class IntegerValue;
class Report;

class BinaryOperatorOptimizer : public NodeVisitor
{
public:
  BinaryOperatorOptimizer( BinaryOperator& op, Report& );

  std::unique_ptr<Expression> optimize();

  void visit_children( Node& ) override;
  void visit_float_value( FloatValue& lhs ) override;
  void visit_integer_value( IntegerValue& lhs ) override;
  void visit_string_value( StringValue& lhs ) override;

private:
  std::unique_ptr<Expression> optimized_result;

  BinaryOperator& op;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BINARYOPERATOROPTIMIZER_H
