#include "NodeVisitor.h"

#include "Argument.h"
#include "BinaryOperator.h"
#include "FloatValue.h"
#include "FunctionCall.h"
#include "FunctionParameterDeclaration.h"
#include "FunctionParameterList.h"
#include "Identifier.h"
#include "IntegerValue.h"
#include "ModuleFunctionDeclaration.h"
#include "Node.h"
#include "StringValue.h"
#include "TopLevelStatements.h"
#include "UnaryOperator.h"
#include "ValueConsumer.h"
#include "VarStatement.h"

namespace Pol::Bscript::Compiler
{

void NodeVisitor::visit_argument( Argument& node )
{
  visit_children( node );
}

void NodeVisitor::visit_binary_operator( BinaryOperator& node )
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

void NodeVisitor::visit_identifier( Identifier& node )
{
  visit_children( node );
}

void NodeVisitor::visit_integer_value( IntegerValue& node )
{
  visit_children( node );
}

void NodeVisitor::visit_module_function_declaration( ModuleFunctionDeclaration& node )
{
  visit_children( node );
}

void NodeVisitor::visit_string_value( StringValue& ) {}

void NodeVisitor::visit_top_level_statements( TopLevelStatements& node )
{
  visit_children( node );
}

void NodeVisitor::visit_unary_operator( UnaryOperator& node )
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
