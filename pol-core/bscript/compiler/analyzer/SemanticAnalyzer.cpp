#include "SemanticAnalyzer.h"

#include "compiler/Report.h"
#include "compiler/ast/Argument.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/FunctionParameterDeclaration.h"
#include "compiler/ast/FunctionParameterList.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/VarStatement.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compiler/model/FunctionLink.h"
#include "compiler/model/Variable.h"

namespace Pol::Bscript::Compiler
{
SemanticAnalyzer::SemanticAnalyzer( Report& report )
  : report( report ),
    globals( Global, report )
{
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::register_const_declarations( CompilerWorkspace& /*workspace*/ )
{
}

void SemanticAnalyzer::analyze( CompilerWorkspace& workspace )
{
  workspace.top_level_statements->accept( *this );

  workspace.global_variable_names = globals.get_names();
}

void SemanticAnalyzer::visit_identifier( Identifier& node )
{
  if ( auto global = globals.find( node.name ) )
  {
    node.variable = global;
  }
  else
  {
    report.error( node, "Unknown identifier '", node.name, "'.\n" );
    return;
  }
}

void SemanticAnalyzer::visit_var_statement( VarStatement& node )
{
  if ( auto existing = globals.find( node.name ) )
  {
    report.error( node, "Global variable '", node.name, "' already defined.\n",
                  "  See also: ", existing->source_location, "\n" );
    return;
  }

  node.variable = globals.create( node.name, 0, false, false, node.source_location );

  visit_children( node );
}

}  // namespace Pol::Bscript::Compiler
