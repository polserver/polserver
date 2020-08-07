#include "NodeVisitor.h"

#include "Argument.h"
#include "ArrayElementAccess.h"
#include "ArrayElementIndexes.h"
#include "ArrayInitializer.h"
#include "BinaryOperator.h"
#include "Block.h"
#include "BreakStatement.h"
#include "ConstDeclaration.h"
#include "ContinueStatement.h"
#include "DebugStatementMarker.h"
#include "DictionaryEntry.h"
#include "DictionaryInitializer.h"
#include "ElvisOperator.h"
#include "EnumDeclaration.h"
#include "ErrorInitializer.h"
#include "ExitStatement.h"
#include "FloatValue.h"
#include "FunctionCall.h"
#include "FunctionParameterDeclaration.h"
#include "FunctionParameterList.h"
#include "GetMember.h"
#include "Identifier.h"
#include "IntegerValue.h"
#include "MethodCall.h"
#include "MethodCallArgumentList.h"
#include "ModuleFunctionDeclaration.h"
#include "Node.h"
#include "ReturnStatement.h"
#include "StringValue.h"
#include "StructInitializer.h"
#include "StructMemberInitializer.h"
#include "TopLevelStatements.h"
#include "UnaryOperator.h"
#include "UninitializedValue.h"
#include "ValueConsumer.h"
#include "VarStatement.h"

namespace Pol::Bscript::Compiler
{

void NodeVisitor::visit_argument( Argument& node )
{
  visit_children( node );
}

void NodeVisitor::visit_array_element_access( ArrayElementAccess& node )
{
  visit_children( node );
}

void NodeVisitor::visit_array_element_indexes( ArrayElementIndexes& node )
{
  visit_children( node );
}

void NodeVisitor::visit_array_initializer( ArrayInitializer& node )
{
  visit_children( node );
}

void NodeVisitor::visit_binary_operator( BinaryOperator& node )
{
  visit_children( node );
}

void NodeVisitor::visit_block( Block& node )
{
  visit_children( node );
}

void NodeVisitor::visit_break_statement( BreakStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_const_declaration( ConstDeclaration& node )
{
  visit_children( node );
}

void NodeVisitor::visit_continue_statement( ContinueStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_debug_statement_marker( DebugStatementMarker& node )
{
  visit_children( node );
}

void NodeVisitor::visit_dictionary_entry( DictionaryEntry& node )
{
  visit_children( node );
}

void NodeVisitor::visit_dictionary_initializer( DictionaryInitializer& node )
{
  visit_children( node );
}

void NodeVisitor::visit_elvis_operator( ElvisOperator& node )
{
  visit_children( node );
}

void NodeVisitor::visit_enum_declaration( EnumDeclaration& node )
{
  visit_children( node );
}

void NodeVisitor::visit_error_initializer( ErrorInitializer& node )
{
  visit_children( node );
}

void NodeVisitor::visit_exit_statement( ExitStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_float_value( FloatValue& node )
{
  visit_children( node );
}

void NodeVisitor::visit_function_call( FunctionCall& node )
{
  visit_children( node );
}

void NodeVisitor::visit_function_parameter_declaration( FunctionParameterDeclaration& node )
{
  visit_children( node );
}

void NodeVisitor::visit_function_parameter_list( FunctionParameterList& node )
{
  visit_children( node );
}

void NodeVisitor::visit_get_member( GetMember& node )
{
  visit_children( node );
}

void NodeVisitor::visit_identifier( Identifier& node )
{
  visit_children( node );
}

void NodeVisitor::visit_integer_value( IntegerValue& node )
{
  visit_children( node );
}

void NodeVisitor::visit_method_call( MethodCall& node )
{
  visit_children( node );
}

void NodeVisitor::visit_method_call_argument_list( MethodCallArgumentList& node )
{
  visit_children( node );
}

void NodeVisitor::visit_module_function_declaration( ModuleFunctionDeclaration& node )
{
  visit_children( node );
}

void NodeVisitor::visit_return_statement( ReturnStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_string_value( StringValue& ) {}

void NodeVisitor::visit_struct_initializer( StructInitializer& node )
{
  visit_children( node );
}

void NodeVisitor::visit_struct_member_initializer( StructMemberInitializer& node )
{
  visit_children( node );
}

void NodeVisitor::visit_top_level_statements( TopLevelStatements& node )
{
  visit_children( node );
}

void NodeVisitor::visit_unary_operator( UnaryOperator& node )
{
  visit_children( node );
}

void NodeVisitor::visit_uninitialized_value( UninitializedValue& node )
{
  visit_children( node );
}

void NodeVisitor::visit_value_consumer( ValueConsumer& node )
{
  visit_children( node );
}

void NodeVisitor::visit_var_statement( VarStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_children( Node& parent )
{
  for ( const auto& child : parent.children )
  {
    child->accept( *this );
  }
}

}  // namespace Pol::Bscript::Compiler
