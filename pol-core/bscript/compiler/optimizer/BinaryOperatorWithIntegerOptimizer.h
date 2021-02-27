#ifndef POLSERVER_BINARYOPERATORWITHINTEGEROPTIMIZER_H
#define POLSERVER_BINARYOPERATORWITHINTEGEROPTIMIZER_H

#include "bscript/compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class Expression;
class Report;

class BinaryOperatorWithIntegerOptimizer : public NodeVisitor
{
public:
  const IntegerValue& lhs;
  const BinaryOperator& op;
  Report& report;

  std::unique_ptr<Expression> optimized_result;

  BinaryOperatorWithIntegerOptimizer( IntegerValue& lhs, BinaryOperator&, Report& );

  void visit_children( Node& parent ) override;
  void visit_integer_value( IntegerValue& rhs ) override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BINARYOPERATORWITHINTEGEROPTIMIZER_H
