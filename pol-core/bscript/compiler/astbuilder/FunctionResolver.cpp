#include "FunctionResolver.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/DefaultConstructorFunction.h"
#include "bscript/compiler/ast/Function.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/astbuilder/AvailableParseTree.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
FunctionResolver::FunctionResolver( Report& report ) : report( report ) {}
FunctionResolver::~FunctionResolver() = default;

void FunctionResolver::force_reference( const ScopableName& name, const SourceLocation& loc )
{
  register_function_link( name, std::make_shared<FunctionLink>( loc, "" /* calling scope */ ) );
}

void FunctionResolver::register_available_user_function(
    const SourceLocation& source_location,
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx, bool force_reference )
{
  ScopableName name( ScopeName::Global, ctx->IDENTIFIER()->getText() );

  // Exported functions are always forced to be referenced.
  register_available_user_function_parse_tree( source_location, ctx, std::move( name ),
                                               force_reference || ctx->EXPORTED() );
}


void FunctionResolver::register_available_scoped_function(
    const SourceLocation& source_location, const ScopeName& class_name,
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx )
{
  ScopableName name( class_name, ctx->IDENTIFIER()->getText() );
  register_available_user_function_parse_tree( source_location, ctx, std::move( name ),
                                               ctx->EXPORTED() );
}

void FunctionResolver::register_available_class_declaration(
    const SourceLocation& loc, const ScopeName& class_name,
    EscriptGrammar::EscriptParser::ClassDeclarationContext* ctx )
{
  register_available_class_decl_parse_tree( loc, ctx, class_name );
}

void FunctionResolver::register_function_link( const ScopableName& name,
                                               std::shared_ptr<FunctionLink> function_link )
{
  Function* resolved_function = nullptr;
  const auto& calling_scope = function_link->calling_scope;
  const auto& unscoped_name = name.name;

  auto resolve_if_existing = [&]( const ScopableName& key )
  {
    resolved_function = check_existing( key );
    if ( resolved_function )
    {
      ( function_link )->link_to( resolved_function );
      report.debug( function_link->source_location, "linking {} to {}::{} [already registered]",
                    name.string(), resolved_function->module_name, resolved_function->name );
      return true;
    }
    return false;
  };

  // If call scope given, check that specific one.
  if ( name.scope.exists() && resolve_if_existing( name ) )
    return;

  // If calling scope is present, check that first, eg. nside Animal class, `foo()` checks
  // `Animal::foo()` first.
  if ( !calling_scope.empty() && resolve_if_existing( { calling_scope, unscoped_name } ) )
    return;

  // Check global scope, only if calling scope is empty
  if ( calling_scope.empty() &&
       resolve_if_existing( ScopableName( ScopeName::Global, unscoped_name ) ) )
    return;

  unresolved_function_links[name].push_back( std::move( function_link ) );
}

std::string FunctionResolver::register_function_expression(
    const SourceLocation& source_location,
    EscriptGrammar::EscriptParser::FunctionExpressionContext* ctx )
{
  auto name = function_expression_name( source_location );
  auto apt = AvailableParseTree{ source_location, ctx, "" };
  available_user_function_parse_trees.insert( { name, apt } );
  return name;
}

void FunctionResolver::register_module_function( ModuleFunctionDeclaration* mf )
{
  const auto& name = mf->name;

  auto itr = available_user_function_parse_trees.find( name );
  if ( itr != available_user_function_parse_trees.end() )
  {
    const auto& previous = ( *itr ).second;

    report.error( previous.source_location,
                  "User Function '{}' conflicts with Module Function of the same name.\n"
                  "  Module Function declaration: {}",
                  name, mf->source_location );
  }

  resolved_functions[{ ScopeName::Global, name }] = mf;
  resolved_functions[{ mf->module_name, name }] = mf;
}

void FunctionResolver::register_user_function( const std::string& scope, UserFunction* uf )
{
  const std::string& name = uf->name;
  resolved_functions[{ scope, name }] = uf;
  report.debug( uf->source_location, "registering uf ({}, {}).", scope, name );

  // Constructors are registered in global scope
  if ( uf->type == UserFunctionType::Constructor )
  {
    resolved_functions[{ ScopeName::Global, name }] = uf;
  }
}

bool FunctionResolver::resolve( std::vector<AvailableParseTree>& to_build_ast )
{
  // Attempt to link all unresolved function links
  for ( auto unresolved_itr = unresolved_function_links.begin();
        unresolved_itr != unresolved_function_links.end(); )
  {
    for ( auto function_link_itr = ( *unresolved_itr ).second.begin();
          function_link_itr != ( *unresolved_itr ).second.end(); )
    {
      auto& function_link = *function_link_itr;
      const auto& calling_scope = function_link->calling_scope;
      const auto& name = ( *unresolved_itr ).first;
      const auto& unscoped_name = name.name;
      const auto& call_scope = name.scope;

      // Given a Function, link the FunctionLink to it and remove it from the list.
      // If the function is already resolved, link it.
      auto resolve_if_existing = [&]( const ScopableName& key )
      {
        auto resolved_function = check_existing( key );
        if ( resolved_function )
        {
          ( *function_link_itr )->link_to( resolved_function );
          function_link_itr = ( *unresolved_itr ).second.erase( function_link_itr );
          report.debug( function_link->source_location, "linking ({}, {}) to {}::{}",
                        call_scope.string(), name.string(), resolved_function->module_name,
                        resolved_function->name );
          return true;
        }
        return false;
      };

      auto advance_if_build_available = [&]( const ScopableName& key )
      {
        if ( build_if_available( to_build_ast, calling_scope, key ) )
        {
          ++function_link_itr;
          return true;
        }
        return false;
      };

      // Link the function if possible, otherwise try to build it.
      auto link_handled = [&]( const ScopableName& key )
      { return resolve_if_existing( key ) || advance_if_build_available( key ); };

      report.debug( function_link->source_location, "resolving {}", name.string() );

      if ( !call_scope.empty() && link_handled( { call_scope, unscoped_name } ) )
        continue;

      // If calling scope is present, check that first, eg. nside Animal class, `foo()` checks
      // `Animal::foo()` first.
      if ( !calling_scope.empty() && link_handled( { calling_scope, unscoped_name } ) )
        continue;

      // Check global scope
      if ( link_handled( { ScopeName::Global, unscoped_name } ) )
        continue;

      // Link was not handled, move to the next one
      ++function_link_itr;
    }

    // If all links were resolved, remove the entry.
    if ( ( *unresolved_itr ).second.empty() )
    {
      unresolved_itr = unresolved_function_links.erase( unresolved_itr );
    }
    else
    {
      // Do not complain here, as the identifier in `[scope::]name` may be a variable.
      // Error handled in SemanticAnalyzer.
      ++unresolved_itr;
    }
  }

  return !to_build_ast.empty();
}

std::string FunctionResolver::function_expression_name( const SourceLocation& source_location )
{
  return fmt::format( "funcexpr@{}:{}:{}", source_location.source_file_identifier->index,
                      source_location.range.start.line_number,
                      source_location.range.start.character_column );
}

void FunctionResolver::register_available_user_function_parse_tree(
    const SourceLocation& source_location, antlr4::ParserRuleContext* ctx,
    // const ScopeName& scope_name, antlr4::tree::TerminalNode* identifier,
    const ScopableName& name, bool force_reference )
{
  const auto& unscoped_name = name.name;
  // ScopableName name( scope_name, unscoped_name );

  // Eg."foo" or "Animal::foo"
  auto scoped_name = name.string();

  // Eg. "", "Animal"
  auto scope = name.scope.string();

  report.debug( source_location, "registering funct apt {}", scoped_name );

  auto itr = available_user_function_parse_trees.find( scoped_name );
  if ( itr != available_user_function_parse_trees.end() )
  {
    AvailableParseTree& previous = ( *itr ).second;

    report.error( source_location,
                  "Function '{}' defined more than once.\n"
                  "  Previous declaration: {}",
                  name.maybe_scoped_string(), previous.source_location );
  }

  auto itr2 = resolved_functions.find( { scope, unscoped_name } );
  if ( itr2 != resolved_functions.end() )
  {
    auto* previous = ( *itr2 ).second;

    std::string what = dynamic_cast<ModuleFunctionDeclaration*>( previous ) == nullptr
                           ? "User Function"
                           : "Module Function";

    report.error( source_location,
                  "User Function '{}' conflicts with {} of the same name.\n"
                  "  {} declaration: {}",
                  name.maybe_scoped_string(), what, what, previous->source_location );
  }

  auto itr3 = available_class_decl_parse_trees.find( scope );
  if ( itr3 != available_class_decl_parse_trees.end() )
  {
    auto previous = ( *itr3 ).second;

    report.error( source_location,
                  "User Function '{}' conflicts with Class of the same name.\n"
                  "  Class declaration: {}",
                  name.maybe_scoped_string(), previous.source_location );
  }

  auto apt = AvailableParseTree{ source_location, ctx, scope };
  available_user_function_parse_trees.insert( { scoped_name, apt } );

  if ( scope == name.name )
  {
    // Constructors are allowed to be referenced in global function scope as well, ie. both
    // `Foo:Foo()` (above) and `Foo()` (here)
    available_user_function_parse_trees.insert( { scope, apt } );
  }

  if ( force_reference )
  {
    // just make sure there is an entry, so that we build an AST for it
    register_function_link(
        name, std::make_shared<FunctionLink>( source_location, scope /* calling scope */ ) );
  }
}

void FunctionResolver::register_available_class_decl_parse_tree(
    const SourceLocation& source_location, antlr4::ParserRuleContext* ctx,
    const ScopeName& scope_name )
{
  const auto name = scope_name.string();
  report.debug( source_location, "registering class apt ({}).", name );
  auto itr = available_user_function_parse_trees.find( name );
  if ( itr != available_user_function_parse_trees.end() )
  {
    AvailableParseTree& previous = ( *itr ).second;

    report.error( source_location,
                  "Class '{}' conflicts with User Function of the same name.\n"
                  "  Previous declaration: {}",
                  name, previous.source_location );
  }

  auto itr2 = available_class_decl_parse_trees.find( name );
  if ( itr2 != available_class_decl_parse_trees.end() )
  {
    AvailableParseTree& previous = ( *itr2 ).second;
    report.error( source_location,
                  "Class '{}' defined more than once.\n"
                  "  Previous declaration: {}",
                  name, previous.source_location );
  }

  auto itr3 = resolved_functions.find( { ScopeName::Global, name } );
  if ( itr3 != resolved_functions.end() )
  {
    auto* previous = ( *itr3 ).second;

    std::string what = dynamic_cast<ModuleFunctionDeclaration*>( previous ) == nullptr
                           ? "User Function"
                           : "Module Function";

    report.error( source_location,
                  "Class '{}' conflicts with {} of the same name.\n"
                  "  {} declaration: {}",
                  name, what, what, previous->source_location );
  }

  auto apt = AvailableParseTree{ source_location, ctx, name };
  available_class_decl_parse_trees.insert( { name, apt } );
}

Function* FunctionResolver::check_existing( const ScopableName& key ) const
{
  auto itr = resolved_functions.find( key );
  if ( itr != resolved_functions.end() )
  {
    return ( *itr ).second;
  }
  return nullptr;
}

bool FunctionResolver::build_if_available( std::vector<AvailableParseTree>& to_build_ast,
                                           const std::string& calling_scope,
                                           const ScopableName& call )
{
  AvailableParseTreeMap::iterator itr;

  // If a call scope was given, _only_ check that one.
  // eg. `Animal::foo()` will only search for `Animal::foo()`, disregarding a possible parent
  // scoped `::foo()`
  if ( call.scoped() )
  {
    auto scoped_call_name = call.string();
    itr = available_user_function_parse_trees.find( scoped_call_name );
    if ( itr != available_user_function_parse_trees.end() )
    {
      to_build_ast.push_back( ( *itr ).second );
      available_user_function_parse_trees.erase( itr );
      return true;
    }

    // Check if there is a class available with that scope, as it could provide this function
    // later.
    itr = available_class_decl_parse_trees.find( call.scope.string() );
    if ( itr != available_class_decl_parse_trees.end() )
    {
      to_build_ast.push_back( ( *itr ).second );
      available_class_decl_parse_trees.erase( itr );
      return true;
    }

    // Nothing found. Can't build a scoped function call.
    return false;
  }

  // Inside a scope, eg. `Animal`...
  if ( !calling_scope.empty() )
  {
    // Check if exists in calling scope, eg. `foo()` checks `Animal::foo()`
    auto scoped_call_name = fmt::format( "{}::{}", calling_scope, call.name );
    itr = available_user_function_parse_trees.find( scoped_call_name );
    if ( itr != available_user_function_parse_trees.end() )
    {
      to_build_ast.push_back( ( *itr ).second );
      available_user_function_parse_trees.erase( itr );
      return true;
    }
  }

  // Check if there is a class available with the function name.
  itr = available_class_decl_parse_trees.find( call.name );
  if ( itr != available_class_decl_parse_trees.end() )
  {
    to_build_ast.push_back( ( *itr ).second );
    available_class_decl_parse_trees.erase( itr );
    return true;
  }

  // Check if exists in global scope.
  itr = available_user_function_parse_trees.find( call.name );
  if ( itr != available_user_function_parse_trees.end() )
  {
    to_build_ast.push_back( ( *itr ).second );
    available_user_function_parse_trees.erase( itr );
    return true;
  }

  return false;
};
}  // namespace Pol::Bscript::Compiler
