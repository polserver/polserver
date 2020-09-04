#ifndef POLSERVER_INSTRUCTIONGENERATOR_H
#define POLSERVER_INSTRUCTIONGENERATOR_H

#include "compiler/ast/NodeVisitor.h"

#include <map>
#include <string>

namespace Pol::Bscript::Compiler
{
class FlowControlLabel;
class InstructionEmitter;
class JumpStatement;

class InstructionGenerator : public NodeVisitor
{
public:
  explicit InstructionGenerator( InstructionEmitter&,
                                 std::map<std::string, FlowControlLabel>& user_function_labels,
                                 bool in_function );

  void generate( Node& );

  void generate_jump_for_statement( JumpStatement& );


  void visit_assign_variable_consume( AssignVariableConsume& ) override;
  void visit_binary_operator( BinaryOperator& ) override;
  void visit_break_statement( BreakStatement& ) override;
  void visit_block( Block& ) override;
  void visit_continue_statement( ContinueStatement& ) override;
  void visit_exit_statement( ExitStatement& ) override;
  void visit_float_value( FloatValue& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_function_parameter_list( FunctionParameterList& ) override;
  void visit_function_parameter_declaration( FunctionParameterDeclaration& ) override;
  void visit_identifier( Identifier& ) override;
  void visit_if_then_else_statement( IfThenElseStatement& ) override;
  void visit_integer_value( IntegerValue& ) override;
  void visit_program( Program& ) override;
  void visit_program_parameter_declaration( ProgramParameterDeclaration& ) override;
  void visit_return_statement( ReturnStatement& ) override;
  void visit_string_value( StringValue& ) override;
  void visit_unary_operator( UnaryOperator& ) override;
  void visit_user_function( UserFunction& ) override;
  void visit_value_consumer( ValueConsumer& ) override;
  void visit_var_statement( VarStatement& ) override;
  void visit_while_loop( WhileLoop& ) override;

private:
  // There are two of these because sometimes when calling a method
  // on InstructionEmitter, the variable name reads better as a noun,
  // and sometimes it reads better as a verb.
  InstructionEmitter& emitter;
  InstructionEmitter& emit;

  std::map<std::string, FlowControlLabel>& user_function_labels;
  const bool in_function;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_INSTRUCTIONGENERATOR_H
