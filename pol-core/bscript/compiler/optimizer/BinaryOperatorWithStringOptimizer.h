#ifndef POLSERVER_BINARYOPERATORWITHSTRINGOPTIMIZER_H
#define POLSERVER_BINARYOPERATORWITHSTRINGOPTIMIZER_H

#include "compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class BinaryOperator;
class Expression;
class StringValue;

class BinaryOperatorWithStringOptimizer : public NodeVisitor
{
public:
  const StringValue& lhs;
  const BinaryOperator& op;

  std::unique_ptr<Expression> optimized_result;

  BinaryOperatorWithStringOptimizer( StringValue& lhs, BinaryOperator& op );

  void visit_children( Node& parent ) override;
  void visit_string_value( StringValue& rhs ) override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BINARYOPERATORWITHSTRINGOPTIMIZER_H
