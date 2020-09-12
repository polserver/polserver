#ifndef POLSERVER_OPTIMIZER_H
#define POLSERVER_OPTIMIZER_H

#include "compiler/ast/NodeVisitor.h"

#include <memory>

#include "compiler/model/UserFunctionInclusion.h"

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Constants;
class Report;

class Optimizer : public NodeVisitor
{
public:
  Optimizer( Constants&, Report& );

  void optimize( CompilerWorkspace&, UserFunctionInclusion );
  void visit_children( Node& ) override;

  void visit_binary_operator( BinaryOperator& ) override;
  void visit_branch_selector( BranchSelector& ) override;
  void visit_const_declaration( ConstDeclaration& ) override;
  void visit_identifier( Identifier& ) override;
  void visit_if_then_else_statement( IfThenElseStatement& ) override;
  void visit_unary_operator( UnaryOperator& ) override;
  void visit_value_consumer( ValueConsumer& ) override;

  std::unique_ptr<Node> optimized_replacement;

private:
  Constants& constants;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_OPTIMIZER_H
