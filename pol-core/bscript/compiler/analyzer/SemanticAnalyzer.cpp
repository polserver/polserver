#include "SemanticAnalyzer.h"

#include "compiler/Report.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
SemanticAnalyzer::SemanticAnalyzer( Report& report )
  : report( report )
{
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::register_const_declarations( CompilerWorkspace& /*workspace*/ )
{
}

void SemanticAnalyzer::analyze( CompilerWorkspace& workspace )
{
  workspace.top_level_statements->accept( *this );
}

void SemanticAnalyzer::visit_var_statement( VarStatement& node )
{
  if ( auto existing = globals.find( node.name ) )
  {
    report.error( node, "Global variable '", node.name, "' already defined.\n" );
    return;
  }

  node.variable = globals.create( node.name, 0, false, false, node.source_location );
  visit_children( node );
}

}  // namespace Pol::Bscript::Compiler
