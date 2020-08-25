#include "NodeVisitor.h"

#include "compiler/ast/Argument.h"
#include "compiler/ast/FloatValue.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/FunctionParameterDeclaration.h"
#include "compiler/ast/FunctionParameterList.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Node.h"
#include "compiler/ast/StringValue.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/ValueConsumer.h"
#include "compiler/ast/VarStatement.h"

namespace Pol::Bscript::Compiler
{
void NodeVisitor::visit_argument( Argument& node )
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

void NodeVisitor::visit_module_function_declaration( ModuleFunctionDeclaration& node )
{
  visit_children( node );
}

void NodeVisitor::visit_string_value( StringValue& node )
{
  visit_children( node );
}

void NodeVisitor::visit_top_level_statements( TopLevelStatements& node )
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
