#include "SemanticAnalyzer.h"

#include <boost/range/adaptor/reversed.hpp>

#include "compiler/Report.h"
#include "compiler/analyzer/Constants.h"
#include "compiler/analyzer/LocalVariableScope.h"
#include "compiler/ast/Argument.h"
#include "compiler/ast/Block.h"
#include "compiler/ast/ConstDeclaration.h"
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
#include "compiler/astbuilder/SimpleValueCloner.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compiler/model/FunctionLink.h"
#include "compiler/model/Variable.h"

namespace Pol::Bscript::Compiler
{
SemanticAnalyzer::SemanticAnalyzer( Constants& constants, Report& report )
  : constants( constants ),
    report( report ),
    globals( VariableScope::Global, report ),
    locals( VariableScope::Local, report ),
    local_scopes( locals, report )
{
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::register_const_declarations( CompilerWorkspace& workspace )
{
  for ( auto& constant : workspace.const_declarations )
  {
    workspace.constants.create( *constant );
  }
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

void SemanticAnalyzer::visit_function_call( FunctionCall& fc )
{
  // here we turn the arguments passed (which can be named or positional)
  // into the final_arguments vector, which is just one parameter per
  // argument, in the correct order.

  typedef std::map<std::string, std::unique_ptr<Expression>> ArgumentList;
  ArgumentList arguments_passed;

  bool any_named = false;

  std::vector<std::unique_ptr<Argument>> arguments = fc.take_arguments();
  auto parameters = fc.parameters();

  for ( auto& arg_unique_ptr : arguments )
  {
    auto& arg = *arg_unique_ptr;
    std::string arg_name = arg.identifier;
    if ( arg_name.empty() )
    {
      if ( any_named )
      {
        report.error( arg, "Unnamed args cannot follow named args.\n" );
        return;
      }
      arg_name = parameters[arguments_passed.size()].get().name;
    }
    else
    {
      any_named = true;
    }
    if ( arguments_passed.find( arg_name ) != arguments_passed.end() )
    {
      report.error( arg, "Parameter '", arg_name, "' passed more than once.\n" );
      return;
    }

    arguments_passed[arg_name] = arg.take_expression();
  }

  std::vector<std::unique_ptr<Node>> final_arguments;

  for ( auto& param_ref : parameters )
  {
    FunctionParameterDeclaration& param = param_ref.get();
    auto itr = arguments_passed.find( param.name );
    if ( itr == arguments_passed.end() )
    {
      if ( auto default_value = param.default_value() )
      {
        SimpleValueCloner cloner( report, default_value->source_location );
        auto final_argument = cloner.clone( *default_value );

        if ( final_argument )
        {
          final_arguments.push_back( std::move( final_argument ) );
        }
        else
        {
          report.error( param, "Unable to create argument from default parameter.\n" );
          return;
        }
      }
      else
      {
        report.error( fc, "Parameter ", param.name, " was not passed, and there is no default.\n" );
        return;
      }
    }
    else
    {
      final_arguments.push_back( std::move( ( *itr ).second ) );
      arguments_passed.erase( itr );
    }
  }

  for ( auto& unused_argument : arguments_passed )
  {
    report.error( fc, "Parameter '", unused_argument.first, "' passed by name to '",
                  fc.method_name, "', which takes no such parameter.");
  }
  if ( !arguments_passed.empty() )
    return;

  if ( arguments.size() > parameters.size() )
  {
    report.error( fc, "Too many arguments.  Expected ", parameters.size(), ", got ",
                  arguments.size(), ".\n" );
    return;
  }

  fc.children = std::move( final_arguments );

  // do this afterwards, so that named parameters will not be looked up as identifiers.
  visit_children( fc );
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
  if ( auto constant = constants.find( node.name ) )
  {
    report.error( node, "Cannot define a variable with the same name as constant '", node.name,
                  "'.\n", "  See also: ", constant->source_location, "\n" );
    return;
  }

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
