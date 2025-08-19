#ifndef POLSERVER_OPTIMIZER_H
#define POLSERVER_OPTIMIZER_H

#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/ScopeName.h"

#include <memory>
#include <optional>

#include "bscript/compiler/model/UserFunctionInclusion.h"

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Constants;
class Report;
class Expression;

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
  void visit_conditional_operator( ConditionalOperator& ) override;
  void visit_elvis_operator( ElvisOperator& ) override;

  void visit_while_loop( WhileLoop& ) override;
  void visit_do_while_loop( DoWhileLoop& ) override;
  void visit_repeat_until_loop( RepeatUntilLoop& ) override;

  std::unique_ptr<Node> optimized_replacement;

private:
  std::optional<bool> branch_decision( Expression* exp ) const;
  Constants& constants;
  Report& report;
  // Initially Global scope. Set only when visiting a ConstDeclaration in
  // visit_const_declaration to the constant's scope. Reset to Global after
  // visiting the constant's children.
  ScopeName current_constant_scope_name;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_OPTIMIZER_H
