#include "SemanticAnalyzer.h"

#include <boost/range/adaptor/reversed.hpp>

#include "compiler/Report.h"
#include "compiler/analyzer/LocalVariableScope.h"
#include "compiler/ast/Argument.h"
#include "compiler/ast/Block.h"
#include "compiler/ast/FunctionBody.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/FunctionParameterDeclaration.h"
#include "compiler/ast/FunctionParameterList.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/ProgramParameterDeclaration.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/ast/VarStatement.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compiler/model/FunctionLink.h"
#include "compiler/model/Variable.h"

namespace Pol::Bscript::Compiler
{
SemanticAnalyzer::SemanticAnalyzer( Report& report )
  : report( report ),
    globals( VariableScope::Global, report ),
    locals( VariableScope::Local, report ),
    local_scopes( locals, report )
{
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::register_const_declarations( CompilerWorkspace& /*workspace*/ )
{
}

void SemanticAnalyzer::analyze( CompilerWorkspace& workspace )
{
  workspace.top_level_statements->accept( *this );
  if ( auto& program = workspace.program )
  {
    program->accept( *this );
  }

  for ( auto& user_function : workspace.user_functions )
  {
    user_function->accept( *this );
  }

  workspace.global_variable_names = globals.get_names();
}

void SemanticAnalyzer::visit_block( Block& block )
{
  LocalVariableScope scope( local_scopes, block.debug_variables );

  visit_children( block );

  block.locals_in_block = scope.get_block_locals();
}

void SemanticAnalyzer::visit_function_parameter_list( FunctionParameterList& node )
{
  for ( auto& child : boost::adaptors::reverse( node.children ) )
  {
    child->accept( *this );
  }
}

void SemanticAnalyzer::visit_function_parameter_declaration( FunctionParameterDeclaration& node )
{
  if ( auto existing = locals.find( node.name ) )
  {
    report.error( node, "Parameter '", node.name, "' already defined.\n" );
    return;
  }

  WarnOn warn_on = node.unused ? WarnOn::IfUsed : WarnOn::IfNotUsed;
  local_scopes.current_local_scope()->create( node.name, warn_on, node.source_location );
}

void SemanticAnalyzer::visit_identifier( Identifier& node )
{
  if ( auto local = locals.find( node.name ) )
  {
    local->mark_used();
    node.variable = local;
  }
  else if ( auto global = globals.find( node.name ) )
  {
    node.variable = global;
  }
  else
  {
    report.error( node, "Unknown identifier '", node.name, "'.\n" );
    return;
  }
}

void SemanticAnalyzer::visit_program( Program& program )
{
  LocalVariableScope scope( local_scopes, program.debug_variables );

  visit_children( program );

  program.locals_in_block = scope.get_block_locals();
}

void SemanticAnalyzer::visit_program_parameter_declaration( ProgramParameterDeclaration& node )
{
  if ( auto existing = locals.find( node.name ) )
  {
    report.error( node, "Parameter '", node.name, "' already defined.\n" );
    return;
  }

  WarnOn warn_on = node.unused ? WarnOn::IfUsed : WarnOn::IfNotUsed;
  local_scopes.current_local_scope()->create( node.name, warn_on, node.source_location );
}

void SemanticAnalyzer::visit_user_function( UserFunction& node )
{
  LocalVariableScope scope( local_scopes, node.debug_variables );

  visit_children( node );
}

void SemanticAnalyzer::visit_var_statement( VarStatement& node )
{
  if ( auto local_scope = local_scopes.current_local_scope() )
  {
    node.variable = local_scope->create( node.name, WarnOn::Never, node.source_location );
  }
  else
  {
    if ( auto existing = globals.find( node.name ) )
    {
      report.error( node, "Global variable '", node.name, "' already defined.\n",
                    "  See also: ", existing->source_location, "\n" );
      return;
    }

    node.variable = globals.create( node.name, 0, WarnOn::Never, node.source_location );
  }
  visit_children( node );
}

}  // namespace Pol::Bscript::Compiler
