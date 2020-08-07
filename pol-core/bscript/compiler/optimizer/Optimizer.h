#ifndef POLSERVER_OPTIMIZER_H
#define POLSERVER_OPTIMIZER_H

#include "compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class Constants;
class Expression;
class Report;
class CompilerWorkspace;

class Optimizer : public NodeVisitor
{
public:
  explicit Optimizer( Constants&, Report& );

  void optimize( CompilerWorkspace& );
  void visit_children( Node& ) override;

  void visit_binary_operator( BinaryOperator& ) override;
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
