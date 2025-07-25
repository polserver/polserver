#pragma once

#include "bscript/compiler/ast/NodeVisitor.h"
#include <memory>

namespace Pol::Bscript::Compiler
{
class Expression;
class Report;

class BinaryOperatorShortCircuitOptimizer : public NodeVisitor
{
public:
  BinaryOperatorShortCircuitOptimizer( Report& );

  std::unique_ptr<Expression> optimized_result;

  void visit_children( Node& ) override;
  void visit_binary_operator( BinaryOperator& ) override;
};

}  // namespace Pol::Bscript::Compiler
