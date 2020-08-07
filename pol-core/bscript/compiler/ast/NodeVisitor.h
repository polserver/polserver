#ifndef POLSERVER_NODEVISITOR_H
#define POLSERVER_NODEVISITOR_H

namespace Pol::Bscript::Compiler
{
class Argument;
class ArrayElementAccess;
class ArrayElementIndexes;
class ArrayInitializer;
class BasicForLoop;
class BinaryOperator;
class Block;
class BreakStatement;
class ConstDeclaration;
class ContinueStatement;
class CstyleForLoop;
class DebugStatementMarker;
class DictionaryEntry;
class DictionaryInitializer;
class ElvisOperator;
class ErrorInitializer;
class ExitStatement;
class FloatValue;
class EnumDeclaration;
class FunctionCall;
class FunctionParameterDeclaration;
class FunctionParameterList;
class Identifier;
class IfThenElseStatement;
class IntegerValue;
class GetMember;
class MethodCall;
class MethodCallArgumentList;
class ModuleFunctionDeclaration;
class Node;
class ReturnStatement;
class StringValue;
class StructInitializer;
class StructMemberInitializer;
class TopLevelStatements;
class UnaryOperator;
class UninitializedValue;
class ValueConsumer;
class VarStatement;

class NodeVisitor
{
public:
  virtual ~NodeVisitor() = default;

  virtual void visit_argument( Argument& );
  virtual void visit_array_element_access( ArrayElementAccess& );
  virtual void visit_array_element_indexes( ArrayElementIndexes& );
  virtual void visit_array_initializer( ArrayInitializer& );
  virtual void visit_basic_for_loop( BasicForLoop& );
  virtual void visit_binary_operator( BinaryOperator& );
  virtual void visit_block( Block& );
  virtual void visit_break_statement( BreakStatement& );
  virtual void visit_const_declaration( ConstDeclaration& );
  virtual void visit_continue_statement( ContinueStatement& );
  virtual void visit_cstyle_for_loop( CstyleForLoop& );
  virtual void visit_debug_statement_marker( DebugStatementMarker& );
  virtual void visit_dictionary_entry( DictionaryEntry& );
  virtual void visit_dictionary_initializer( DictionaryInitializer& );
  virtual void visit_elvis_operator( ElvisOperator& );
  virtual void visit_enum_declaration( EnumDeclaration& );
  virtual void visit_error_initializer( ErrorInitializer& );
  virtual void visit_exit_statement( ExitStatement& );
  virtual void visit_float_value( FloatValue& );
  virtual void visit_function_call( FunctionCall& );
  virtual void visit_function_parameter_declaration( FunctionParameterDeclaration& );
  virtual void visit_function_parameter_list( FunctionParameterList& );
  virtual void visit_identifier( Identifier& );
  virtual void visit_get_member( GetMember& );
  virtual void visit_if_then_else_statement( IfThenElseStatement& );
  virtual void visit_integer_value( IntegerValue& );
  virtual void visit_method_call( MethodCall& );
  virtual void visit_method_call_argument_list( MethodCallArgumentList& );
  virtual void visit_module_function_declaration( ModuleFunctionDeclaration& );
  virtual void visit_return_statement( ReturnStatement& );
  virtual void visit_string_value( StringValue& );
  virtual void visit_struct_initializer( StructInitializer& );
  virtual void visit_struct_member_initializer( StructMemberInitializer& );
  virtual void visit_top_level_statements( TopLevelStatements& );
  virtual void visit_unary_operator( UnaryOperator& );
  virtual void visit_uninitialized_value( UninitializedValue& );
  virtual void visit_value_consumer( ValueConsumer& );
  virtual void visit_var_statement( VarStatement& );

  virtual void visit_children( Node& parent );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_NODEVISITOR_H
