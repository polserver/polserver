#include "NodeVisitor.h"

#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/ArrayInitializer.h"
#include "bscript/compiler/ast/BasicForLoop.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/BranchSelector.h"
#include "bscript/compiler/ast/CaseDispatchDefaultSelector.h"
#include "bscript/compiler/ast/CaseDispatchGroup.h"
#include "bscript/compiler/ast/CaseDispatchGroups.h"
#include "bscript/compiler/ast/CaseDispatchSelectors.h"
#include "bscript/compiler/ast/CaseStatement.h"
#include "bscript/compiler/ast/ConditionalOperator.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/CstyleForLoop.h"
#include "bscript/compiler/ast/DebugStatementMarker.h"
#include "bscript/compiler/ast/DictionaryEntry.h"
#include "bscript/compiler/ast/DictionaryInitializer.h"
#include "bscript/compiler/ast/DoWhileLoop.h"
#include "bscript/compiler/ast/ElementAccess.h"
#include "bscript/compiler/ast/ElementAssignment.h"
#include "bscript/compiler/ast/ElementIndexes.h"
#include "bscript/compiler/ast/ElvisOperator.h"
#include "bscript/compiler/ast/EmptyStatement.h"
#include "bscript/compiler/ast/EnumDeclaration.h"
#include "bscript/compiler/ast/ErrorInitializer.h"
#include "bscript/compiler/ast/ExitStatement.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/ForeachLoop.h"
#include "bscript/compiler/ast/FormatExpression.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/IfThenElseStatement.h"
#include "bscript/compiler/ast/InterpolateString.h"
#include "bscript/compiler/ast/JumpStatement.h"
#include "bscript/compiler/ast/MemberAccess.h"
#include "bscript/compiler/ast/MemberAssignment.h"
#include "bscript/compiler/ast/MemberAssignmentByOperator.h"
#include "bscript/compiler/ast/MethodCall.h"
#include "bscript/compiler/ast/MethodCallArgumentList.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/ProgramParameterDeclaration.h"
#include "bscript/compiler/ast/ProgramParameterList.h"
#include "bscript/compiler/ast/RepeatUntilLoop.h"
#include "bscript/compiler/ast/ReturnStatement.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/StructInitializer.h"
#include "bscript/compiler/ast/StructMemberInitializer.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UnaryOperator.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/ValueConsumer.h"
#include "bscript/compiler/ast/VarStatement.h"
#include "bscript/compiler/ast/VariableAssignmentStatement.h"
#include "bscript/compiler/ast/WhileLoop.h"

namespace Pol::Bscript::Compiler
{
void NodeVisitor::visit_argument( Argument& node )
{
  visit_children( node );
}

void NodeVisitor::visit_array_initializer( ArrayInitializer& node )
{
  visit_children( node );
}

void NodeVisitor::visit_basic_for_loop( BasicForLoop& node )
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

void NodeVisitor::visit_branch_selector( BranchSelector& node )
{
  visit_children( node );
}

void NodeVisitor::visit_case_statement( CaseStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_case_dispatch_default_selector( CaseDispatchDefaultSelector& node )
{
  visit_children( node );
}

void NodeVisitor::visit_case_dispatch_group( CaseDispatchGroup& node )
{
  visit_children( node );
}

void NodeVisitor::visit_case_dispatch_groups( CaseDispatchGroups& node )
{
  visit_children( node );
}

void NodeVisitor::visit_case_dispatch_selectors( CaseDispatchSelectors& node )
{
  visit_children( node );
}

void NodeVisitor::visit_const_declaration( ConstDeclaration& node )
{
  visit_children( node );
}

void NodeVisitor::visit_cstyle_for_loop( CstyleForLoop& node )
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

void NodeVisitor::visit_do_while_loop( DoWhileLoop& node )
{
  visit_children( node );
}

void NodeVisitor::visit_element_access( ElementAccess& node )
{
  visit_children( node );
}

void NodeVisitor::visit_element_assignment( ElementAssignment& node )
{
  visit_children( node );
}

void NodeVisitor::visit_element_indexes( ElementIndexes& node )
{
  visit_children( node );
}

void NodeVisitor::visit_elvis_operator( ElvisOperator& node )
{
  visit_children( node );
}

void NodeVisitor::visit_empty_statement( EmptyStatement& node )
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

void NodeVisitor::visit_exit_statement( ExitStatement& ) {}

void NodeVisitor::visit_float_value( FloatValue& ) {}

void NodeVisitor::visit_foreach_loop( ForeachLoop& node )
{
  visit_children( node );
}

void NodeVisitor::visit_function_body( FunctionBody& node )
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

void NodeVisitor::visit_function_reference( FunctionReference& ) {}

void NodeVisitor::visit_identifier( Identifier& ) {}

void NodeVisitor::visit_if_then_else_statement( IfThenElseStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_integer_value( IntegerValue& ) {}

void NodeVisitor::visit_jump_statement( JumpStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_member_access( MemberAccess& node )
{
  visit_children( node );
}

void NodeVisitor::visit_member_assignment( MemberAssignment& node )
{
  visit_children( node );
}

void NodeVisitor::visit_member_assignment_by_operator( MemberAssignmentByOperator& node )
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

void NodeVisitor::visit_program( Program& node )
{
  visit_children( node );
}

void NodeVisitor::visit_program_parameter_declaration( ProgramParameterDeclaration& node )
{
  visit_children( node );
}

void NodeVisitor::visit_program_parameter_list( ProgramParameterList& node )
{
  visit_children( node );
}

void NodeVisitor::visit_repeat_until_loop( RepeatUntilLoop& node )
{
  visit_children( node );
}

void NodeVisitor::visit_return_statement( ReturnStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_string_value( StringValue& ) {}

void NodeVisitor::visit_interpolate_string( InterpolateString& node )
{
  visit_children( node );
}

void NodeVisitor::visit_format_expression( FormatExpression& node )
{
  visit_children( node );
}

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

void NodeVisitor::visit_uninitialized_value( UninitializedValue& ) {}

void NodeVisitor::visit_user_function( UserFunction& node )
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

void NodeVisitor::visit_variable_assignment_statement( VariableAssignmentStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_while_loop( WhileLoop& node )
{
  visit_children( node );
}

void NodeVisitor::visit_conditional_operator( ConditionalOperator& node )
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
