#pragma once

#include "bscript/compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class Expression;
class Report;
class CompilerWorkspace;

class ShortCircuitReporter : public NodeVisitor
{
public:
  ShortCircuitReporter( Report&, bool analysis_mode );

  void visit_binary_operator( BinaryOperator& ) override;
  void visit_element_assignment( ElementAssignment& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_member_assignment( MemberAssignment& ) override;
  void visit_member_assignment_by_operator( MemberAssignmentByOperator& ) override;
  void visit_method_call( MethodCall& ) override;
  void visit_unary_operator( UnaryOperator& ) override;
  void visit_variable_assignment_statement( VariableAssignmentStatement& ) override;

private:
  Report& report;
  // vscode-escript shows the warning at its location, so the source line would be noise.
  const bool analysis_mode;
  void trigger( Node& op );
};

class ShortCircuitWarning : public NodeVisitor
{
public:
  ShortCircuitWarning( Report&, bool analysis_mode );
  void warn( CompilerWorkspace& workspace );

  void visit_binary_operator_short_circuit( BinaryOperatorShortCircuit& ) override;

private:
  ShortCircuitReporter visitor;
};
}  // namespace Pol::Bscript::Compiler
