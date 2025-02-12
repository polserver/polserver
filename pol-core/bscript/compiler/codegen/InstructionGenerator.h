#ifndef POLSERVER_INSTRUCTIONGENERATOR_H
#define POLSERVER_INSTRUCTIONGENERATOR_H

#include "bscript/compiler/ast/NodeVisitor.h"

#include <map>
#include <stack>
#include <string>

namespace Pol::Bscript::Compiler
{
class FlowControlLabel;
class InstructionEmitter;
class SourceLocation;
class Variable;

class InstructionGenerator : public NodeVisitor
{
public:
  InstructionGenerator( InstructionEmitter&,
                        std::map<std::string, FlowControlLabel>& user_function_labels,
                        const std::map<std::string, size_t>& class_declaration_indexes );

  void generate( Node& );
  void generate_default_parameters( const UserFunction& );

  void update_debug_location( const Node& );
  void update_debug_location( const SourceLocation& );

  void visit_array_initializer( ArrayInitializer& ) override;
  void visit_basic_for_loop( BasicForLoop& loop ) override;
  void visit_case_statement( CaseStatement& ) override;
  void visit_cstyle_for_loop( CstyleForLoop& loop ) override;
  void visit_binary_operator( BinaryOperator& ) override;
  void visit_block( Block& ) override;
  void visit_boolean_value( BooleanValue& ) override;
  void visit_branch_selector( BranchSelector& ) override;
  void visit_class_instance( ClassInstance& ) override;
  void visit_debug_statement_marker( DebugStatementMarker& ) override;
  void visit_dictionary_entry( DictionaryEntry& ) override;
  void visit_dictionary_initializer( DictionaryInitializer& ) override;
  void visit_do_while_loop( DoWhileLoop& ) override;
  void visit_element_access( ElementAccess& ) override;
  void visit_element_assignment( ElementAssignment& ) override;
  void visit_elvis_operator( ElvisOperator& elvis ) override;
  void visit_error_initializer( ErrorInitializer& ) override;
  void visit_exit_statement( ExitStatement& ) override;
  void visit_float_value( FloatValue& ) override;
  void visit_foreach_loop( ForeachLoop& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_function_parameter_list( FunctionParameterList& ) override;
  void visit_function_parameter_declaration( FunctionParameterDeclaration& ) override;
  void visit_function_expression( FunctionExpression& ) override;
  void visit_function_reference( FunctionReference& ) override;
  void visit_identifier( Identifier& ) override;
  void visit_if_then_else_statement( IfThenElseStatement& ) override;
  void visit_integer_value( IntegerValue& ) override;
  void visit_jump_statement( JumpStatement& ) override;
  void visit_member_access( MemberAccess& member_access ) override;
  void visit_member_assignment( MemberAssignment& ) override;
  void visit_member_assignment_by_operator( MemberAssignmentByOperator& ) override;
  void visit_method_call( MethodCall& member_call ) override;
  void visit_program( Program& ) override;
  void visit_program_parameter_declaration( ProgramParameterDeclaration& ) override;
  void visit_repeat_until_loop( RepeatUntilLoop& repeat_until ) override;
  void visit_return_statement( ReturnStatement& ) override;
  void visit_spread_element( SpreadElement& ) override;
  void visit_string_value( StringValue& ) override;
  void visit_struct_initializer( StructInitializer& ) override;
  void visit_struct_member_initializer( StructMemberInitializer& ) override;
  void visit_unary_operator( UnaryOperator& ) override;
  void visit_uninitialized_value( UninitializedValue& ) override;
  void visit_user_function( UserFunction& ) override;
  void visit_unpacking_list( UnpackingList& ) override;
  void visit_value_consumer( ValueConsumer& ) override;
  void visit_var_statement( VarStatement& ) override;
  void visit_variable_assignment_statement( VariableAssignmentStatement& ) override;
  void visit_while_loop( WhileLoop& ) override;
  void visit_index_unpacking( IndexUnpacking& ) override;
  void visit_interpolate_string( InterpolateString& ) override;
  void visit_format_expression( FormatExpression& ) override;
  void visit_conditional_operator( ConditionalOperator& ) override;

private:
  void emit_access_variable( Variable& );

  // There are two of these because sometimes when calling a method
  // on InstructionEmitter, the variable name reads better as a noun,
  // and sometimes it reads better as a verb.
  InstructionEmitter& emitter;
  InstructionEmitter& emit;

  std::map<std::string, FlowControlLabel>& user_function_labels;
  const std::map<std::string, size_t>& class_declaration_indexes;
  std::stack<UserFunction*> user_functions;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_INSTRUCTIONGENERATOR_H
