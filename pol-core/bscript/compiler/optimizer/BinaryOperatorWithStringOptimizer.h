#ifndef POLSERVER_BINARYOPERATORWITHSTRINGOPTIMIZER_H
#define POLSERVER_BINARYOPERATORWITHSTRINGOPTIMIZER_H

#include "bscript/compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class Expression;

class BinaryOperatorWithStringOptimizer : public NodeVisitor
{
public:
  const StringValue& lhs;
  const BinaryOperator& op;

  std::unique_ptr<Expression> optimized_result;

  BinaryOperatorWithStringOptimizer( StringValue& lhs, BinaryOperator& op );

  void visit_children( Node& parent ) override;
  void visit_string_value( StringValue& rhs ) override;
  void visit_float_value( FloatValue& rhs ) override;
  void visit_integer_value( IntegerValue& rhs ) override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BINARYOPERATORWITHSTRINGOPTIMIZER_H
