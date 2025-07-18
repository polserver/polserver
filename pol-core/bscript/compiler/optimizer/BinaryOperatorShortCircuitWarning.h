#pragma once

#include "bscript/compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class Expression;
class Report;

// TODO in analyzer would be better
class BinaryOperatorShortCircuitChangeWarning : public NodeVisitor
{
public:
  BinaryOperatorShortCircuitChangeWarning( Report& );

  // void visit_argument( Argument& ) override;
  // void visit_array_initializer( ArrayInitializer& ) override;
  // void visit_basic_for_loop( BasicForLoop& ) override;
  void visit_binary_operator( BinaryOperator& ) override;
  // void visit_binding_statement( BindingStatement& ) override;
  // void visit_block( Block& ) override;
  // void visit_boolean_value( BooleanValue& ) override;
  // void visit_branch_selector( BranchSelector& ) override;
  // void visit_case_statement( CaseStatement& ) override;
  // void visit_case_dispatch_default_selector( CaseDispatchDefaultSelector& ) override;
  // void visit_case_dispatch_group( CaseDispatchGroup& ) override;
  // void visit_case_dispatch_groups( CaseDispatchGroups& ) override;
  // void visit_case_dispatch_selectors( CaseDispatchSelectors& ) override;
  // void visit_const_declaration( ConstDeclaration& ) override;
  // void visit_cstyle_for_loop( CstyleForLoop& ) override;
  // void visit_class_body( ClassBody& ) override;
  // void visit_class_declaration( ClassDeclaration& ) override;
  // void visit_class_instance( ClassInstance& ) override;
  // void visit_class_parameter_declaration( ClassParameterDeclaration& ) override;
  // void visit_class_parameter_list( ClassParameterList& ) override;
  // void visit_debug_statement_marker( DebugStatementMarker& ) override;
  // void visit_dictionary_entry( DictionaryEntry& ) override;
  // void visit_dictionary_initializer( DictionaryInitializer& ) override;
  // void visit_do_while_loop( DoWhileLoop& ) override;
  // void visit_element_access( ElementAccess& ) override;
  void visit_element_assignment( ElementAssignment& ) override;
  // void visit_element_indexes( ElementIndexes& ) override;
  // void visit_elvis_operator( ElvisOperator& ) override;
  // void visit_empty_statement( EmptyStatement& ) override;
  // void visit_enum_declaration( EnumDeclaration& ) override;
  // void visit_error_initializer( ErrorInitializer& ) override;
  // void visit_exit_statement( ExitStatement& ) override;
  // void visit_float_value( FloatValue& ) override;
  // void visit_foreach_loop( ForeachLoop& ) override;
  // void visit_function_body( FunctionBody& ) override;
  void visit_function_call( FunctionCall& ) override;
  // void visit_function_parameter_declaration( FunctionParameterDeclaration& ) override;
  // void visit_function_parameter_list( FunctionParameterList& ) override;
  // void visit_function_expression( FunctionExpression& ) override;
  // void visit_function_reference( FunctionReference& ) override;
  // void visit_generated_function( GeneratedFunction& ) override;
  // void visit_identifier( Identifier& ) override;
  // void visit_if_then_else_statement( IfThenElseStatement& ) override;
  // void visit_integer_value( IntegerValue& ) override;
  // void visit_index_binding( IndexBinding& ) override;
  // void visit_jump_statement( JumpStatement& ) override;
  // void visit_member_access( MemberAccess& ) override;
  void visit_member_assignment( MemberAssignment& ) override;
  void visit_member_assignment_by_operator( MemberAssignmentByOperator& ) override;
  void visit_method_call( MethodCall& ) override;
  // void visit_method_call_argument_list( MethodCallArgumentList& ) override;
  // void visit_module_function_declaration( ModuleFunctionDeclaration& ) override;
  // void visit_program( Program& ) override;
  // void visit_program_parameter_declaration( ProgramParameterDeclaration& ) override;
  // void visit_program_parameter_list( ProgramParameterList& ) override;
  // void visit_repeat_until_loop( RepeatUntilLoop& ) override;
  // void visit_return_statement( ReturnStatement& ) override;
  // void visit_sequence_binding( SequenceBinding& ) override;
  // void visit_spread_element( SpreadElement& ) override;
  // void visit_string_value( StringValue& ) override;
  // void visit_interpolate_string( InterpolateString& ) override;
  // void visit_format_expression( FormatExpression& ) override;
  // void visit_struct_initializer( StructInitializer& ) override;
  // void visit_struct_member_initializer( StructMemberInitializer& ) override;
  // void visit_top_level_statements( TopLevelStatements& ) override;
  void visit_unary_operator( UnaryOperator& ) override;
  // void visit_uninitialized_value( UninitializedValue& ) override;
  // void visit_user_function( UserFunction& ) override;
  // void visit_value_consumer( ValueConsumer& ) override;
  // void visit_var_statement( VarStatement& ) override;
  void visit_variable_assignment_statement( VariableAssignmentStatement& ) override;
  // void visit_variable_binding( VariableBinding& ) override;
  // void visit_while_loop( WhileLoop& ) override;
  // void visit_conditional_operator( ConditionalOperator& ) override;
  // void visit_constant_loop( ConstantPredicateLoop& ) override;
  // void visit_binary_operator_short_circuit( BinaryOperatorShortCircuit& ) override;

private:
  Report& report;
  void trigger( Node& op );
};

class BinaryOperatorShortCircuitWarning : public NodeVisitor
{
public:
  BinaryOperatorShortCircuitWarning( Report& );

  void visit_binary_operator_short_circuit( BinaryOperatorShortCircuit& ) override;

private:
  Report& report;
  BinaryOperatorShortCircuitChangeWarning visitor;
};


}  // namespace Pol::Bscript::Compiler
