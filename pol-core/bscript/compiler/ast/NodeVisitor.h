#ifndef POLSERVER_NODEVISITOR_H
#define POLSERVER_NODEVISITOR_H

namespace Pol::Bscript::Compiler
{
class Argument;
class ArrayInitializer;
class VariableAssignmentStatement;
class BasicForLoop;
class BinaryOperator;
class Block;
class BranchSelector;
class CaseStatement;
class CaseDispatchDefaultSelector;
class CaseDispatchGroup;
class CaseDispatchGroups;
class CaseDispatchSelectors;
class ConstDeclaration;
class CstyleForLoop;
class DebugStatementMarker;
class DictionaryEntry;
class DictionaryInitializer;
class DoWhileLoop;
class ElementAccess;
class ElementAssignment;
class ElementIndexes;
class ElvisOperator;
class EmptyStatement;
class EnumDeclaration;
class ErrorInitializer;
class ExitStatement;
class FloatValue;
class ForeachLoop;
class FunctionBody;
class FunctionCall;
class FunctionParameterDeclaration;
class FunctionParameterList;
class FunctionReference;
class Identifier;
class IfThenElseStatement;
class IntegerValue;
class JumpStatement;
class MemberAccess;
class MethodCall;
class MethodCallArgumentList;
class ModuleFunctionDeclaration;
class Node;
class Program;
class ProgramParameterDeclaration;
class ProgramParameterList;
class RepeatUntilLoop;
class ReturnStatement;
class MemberAssignment;
class MemberAssignmentByOperator;
class StringValue;
class StructInitializer;
class StructMemberInitializer;
class TopLevelStatements;
class UnaryOperator;
class UninitializedValue;
class UserFunction;
class ValueConsumer;
class VarStatement;
class WhileLoop;

class NodeVisitor
{
public:
  virtual ~NodeVisitor() = default;

  virtual void visit_argument( Argument& );
  virtual void visit_array_initializer( ArrayInitializer& );
  virtual void visit_basic_for_loop( BasicForLoop& );
  virtual void visit_binary_operator( BinaryOperator& );
  virtual void visit_block( Block& );
  virtual void visit_branch_selector( BranchSelector& );
  virtual void visit_case_statement( CaseStatement& );
  virtual void visit_case_dispatch_default_selector( CaseDispatchDefaultSelector& );
  virtual void visit_case_dispatch_group( CaseDispatchGroup& );
  virtual void visit_case_dispatch_groups( CaseDispatchGroups& );
  virtual void visit_case_dispatch_selectors( CaseDispatchSelectors& );
  virtual void visit_const_declaration( ConstDeclaration& );
  virtual void visit_cstyle_for_loop( CstyleForLoop& );
  virtual void visit_debug_statement_marker( DebugStatementMarker& );
  virtual void visit_dictionary_entry( DictionaryEntry& );
  virtual void visit_dictionary_initializer( DictionaryInitializer& );
  virtual void visit_do_while_loop( DoWhileLoop& );
  virtual void visit_element_access( ElementAccess& );
  virtual void visit_element_assignment( ElementAssignment& );
  virtual void visit_element_indexes( ElementIndexes& );
  virtual void visit_elvis_operator( ElvisOperator& );
  virtual void visit_empty_statement( EmptyStatement& );
  virtual void visit_enum_declaration( EnumDeclaration& );
  virtual void visit_error_initializer( ErrorInitializer& );
  virtual void visit_exit_statement( ExitStatement& );
  virtual void visit_float_value( FloatValue& );
  virtual void visit_foreach_loop( ForeachLoop& );
  virtual void visit_function_body( FunctionBody& );
  virtual void visit_function_call( FunctionCall& );
  virtual void visit_function_parameter_declaration( FunctionParameterDeclaration& );
  virtual void visit_function_parameter_list( FunctionParameterList& );
  virtual void visit_function_reference( FunctionReference& );
  virtual void visit_identifier( Identifier& );
  virtual void visit_if_then_else_statement( IfThenElseStatement& );
  virtual void visit_integer_value( IntegerValue& );
  virtual void visit_jump_statement( JumpStatement& );
  virtual void visit_member_access( MemberAccess& );
  virtual void visit_member_assignment( MemberAssignment& );
  virtual void visit_member_assignment_by_operator( MemberAssignmentByOperator& );
  virtual void visit_method_call( MethodCall& );
  virtual void visit_method_call_argument_list( MethodCallArgumentList& );
  virtual void visit_module_function_declaration( ModuleFunctionDeclaration& );
  virtual void visit_program( Program& );
  virtual void visit_program_parameter_declaration( ProgramParameterDeclaration& );
  virtual void visit_program_parameter_list( ProgramParameterList& );
  virtual void visit_repeat_until_loop( RepeatUntilLoop& );
  virtual void visit_return_statement( ReturnStatement& );
  virtual void visit_string_value( StringValue& );
  virtual void visit_struct_initializer( StructInitializer& );
  virtual void visit_struct_member_initializer( StructMemberInitializer& );
  virtual void visit_top_level_statements( TopLevelStatements& );
  virtual void visit_unary_operator( UnaryOperator& );
  virtual void visit_uninitialized_value( UninitializedValue& );
  virtual void visit_user_function( UserFunction& );
  virtual void visit_value_consumer( ValueConsumer& );
  virtual void visit_var_statement( VarStatement& );
  virtual void visit_variable_assignment_statement( VariableAssignmentStatement& );
  virtual void visit_while_loop( WhileLoop& );

  virtual void visit_children( Node& parent );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_NODEVISITOR_H
