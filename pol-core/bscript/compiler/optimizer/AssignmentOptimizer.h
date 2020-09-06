#ifndef POLSERVER_ASSIGNMENTOPTIMIZER_H
#define POLSERVER_ASSIGNMENTOPTIMIZER_H

#include <memory>

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
class BinaryOperator;
class Expression;

class AssignmentOptimizer : public NodeVisitor
{
public:
  explicit AssignmentOptimizer( BinaryOperator& assignment );

  std::unique_ptr<Expression> optimize();

  void visit_children( Node& ) override;
  void visit_element_access( ElementAccess& lhs ) override;

private:
  std::unique_ptr<Expression> optimized_result;
  BinaryOperator& assignment;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_ASSIGNMENTOPTIMIZER_H
