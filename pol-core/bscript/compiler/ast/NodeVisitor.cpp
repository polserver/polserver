#include "NodeVisitor.h"

#include "compiler/ast/Argument.h"
#include "compiler/ast/ArrayInitializer.h"
#include "compiler/ast/AssignVariableConsume.h"
#include "compiler/ast/BasicForLoop.h"
#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/Block.h"
#include "compiler/ast/BranchSelector.h"
#include "compiler/ast/CaseDispatchDefaultSelector.h"
#include "compiler/ast/CaseDispatchGroup.h"
#include "compiler/ast/CaseDispatchGroups.h"
#include "compiler/ast/CaseDispatchSelectors.h"
#include "compiler/ast/CaseStatement.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/CstyleForLoop.h"
#include "compiler/ast/DictionaryEntry.h"
#include "compiler/ast/DictionaryInitializer.h"
#include "compiler/ast/DoWhileLoop.h"
#include "compiler/ast/ElementAccess.h"
#include "compiler/ast/ElementAssignment.h"
#include "compiler/ast/ElementIndexes.h"
#include "compiler/ast/ElvisOperator.h"
#include "compiler/ast/EmptyStatement.h"
#include "compiler/ast/EnumDeclaration.h"
#include "compiler/ast/ErrorInitializer.h"
#include "compiler/ast/ExitStatement.h"
#include "compiler/ast/FloatValue.h"
#include "compiler/ast/ForeachLoop.h"
#include "compiler/ast/FunctionBody.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/FunctionParameterDeclaration.h"
#include "compiler/ast/FunctionParameterList.h"
#include "compiler/ast/GetMember.h"
#include "compiler/ast/IfThenElseStatement.h"
#include "compiler/ast/JumpStatement.h"
#include "compiler/ast/MethodCall.h"
#include "compiler/ast/MethodCallArgumentList.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Node.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/ProgramParameterDeclaration.h"
#include "compiler/ast/ProgramParameterList.h"
#include "compiler/ast/RepeatUntilLoop.h"
#include "compiler/ast/ReturnStatement.h"
#include "compiler/ast/SetMember.h"
#include "compiler/ast/SetMemberByOperator.h"
#include "compiler/ast/StringValue.h"
#include "compiler/ast/StructInitializer.h"
#include "compiler/ast/StructMemberInitializer.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/UnaryOperator.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/ast/ValueConsumer.h"
#include "compiler/ast/VarStatement.h"
#include "compiler/ast/WhileLoop.h"

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

void NodeVisitor::visit_assign_variable_consume( AssignVariableConsume& node )
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

void NodeVisitor::visit_exit_statement( ExitStatement& )
{
}

void NodeVisitor::visit_float_value( FloatValue& )
{
}

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

void NodeVisitor::visit_function_reference( FunctionReference& )
{
}

void NodeVisitor::visit_get_member( GetMember& node )
{
  visit_children( node );
}

void NodeVisitor::visit_identifier( Identifier& )
{
}

void NodeVisitor::visit_if_then_else_statement( IfThenElseStatement& node )
{
  visit_children( node );
}

void NodeVisitor::visit_integer_value( IntegerValue& )
{
}

void NodeVisitor::visit_method_call( MethodCall& node )
{
  visit_children( node );
}

void NodeVisitor::visit_method_call_argument_list( MethodCallArgumentList& node )
{
  visit_children( node );
}

void NodeVisitor::visit_jump_statement( JumpStatement& node )
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

void NodeVisitor::visit_set_member( SetMember& node )
{
  visit_children( node );
}

void NodeVisitor::visit_set_member_by_operator( SetMemberByOperator& node )
{
  visit_children( node );
}

void NodeVisitor::visit_string_value( StringValue& )
{
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

void NodeVisitor::visit_uninitialized_value( UninitializedValue& )
{
}

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

void NodeVisitor::visit_while_loop( WhileLoop& node )
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
