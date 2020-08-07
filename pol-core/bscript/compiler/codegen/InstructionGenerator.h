#ifndef POLSERVER_INSTRUCTIONGENERATOR_H
#define POLSERVER_INSTRUCTIONGENERATOR_H

#include "compiler/ast/NodeVisitor.h"

#include <map>
#include <string>

namespace Pol::Bscript::Compiler
{
class AssignSubscript;
class AssignVariableConsume;
class InstructionEmitter;
class FlowControlLabel;
class JumpStatement;
class Program;
class ProgramParameterDeclaration;
class SetMember;
class SetMemberByOperator;
class SourceLocation;
class UserFunction;

class InstructionGenerator : public NodeVisitor
{
public:
  InstructionGenerator( InstructionEmitter&,
                        std::map<std::string, FlowControlLabel>& user_function_labels,
                        bool in_function);

  void generate( Node& node );

  void update_debug_location( const Node& );
  void update_debug_location( const SourceLocation& );

  void generate_jump_for_statement( JumpStatement& );

  void visit_array_element_access( ArrayElementAccess& ) override;
  void visit_array_initializer( ArrayInitializer& ) override;
  void visit_assign_subscript( AssignSubscript& ) override;
  void visit_assign_variable_consume( AssignVariableConsume& ) override;
  void visit_basic_for_loop( BasicForLoop& loop ) override;
  void visit_case_statement( CaseStatement& ) override;
  void visit_cstyle_for_loop( CstyleForLoop& loop ) override;
  void visit_identifier( Identifier& ) override;
  void visit_binary_operator( BinaryOperator& ) override;
  void visit_block( Block& ) override;
  void visit_break_statement( BreakStatement& ) override;
  void visit_continue_statement( ContinueStatement& ) override;
  void visit_debug_statement_marker( DebugStatementMarker& ) override;
  void visit_dictionary_initializer( DictionaryInitializer& ) override;
  void visit_dictionary_entry( DictionaryEntry& entry ) override;
  void visit_do_while_loop( DoWhileLoop& ) override;
  void visit_elvis_operator( ElvisOperator& elvis ) override;
  void visit_error_initializer( ErrorInitializer& ) override;
  void visit_exit_statement( ExitStatement& ) override;
  void visit_if_then_else_statement( IfThenElseStatement& ) override;
  void visit_float_value( FloatValue& ) override;
  void visit_integer_value( IntegerValue& ) override;
  void visit_foreach_loop( ForeachLoop& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_function_parameter_list( FunctionParameterList& ) override;
  void visit_function_parameter_declaration( FunctionParameterDeclaration& ) override;
  void visit_function_reference( FunctionReference& ) override;
  void visit_get_member( GetMember& member_access ) override;
  void visit_method_call( MethodCall& member_call ) override;
  void visit_module_function_declaration( ModuleFunctionDeclaration& ) override;
  void visit_program( Program& ) override;
  void visit_program_parameter_declaration( ProgramParameterDeclaration& ) override;
  void visit_repeat_until_loop( RepeatUntilLoop& repeat_until ) override;
  void visit_return_statement( ReturnStatement& ) override;
  void visit_set_member( SetMember& ) override;
  void visit_set_member_by_operator( SetMemberByOperator& ) override;
  void visit_string_value( StringValue& ) override;
  void visit_struct_initializer( StructInitializer& ) override;
  void visit_struct_member_initializer( StructMemberInitializer& ) override;
  void visit_unary_operator( UnaryOperator& unary_operator ) override;
  void visit_uninitialized_value( UninitializedValue& ) override;
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
