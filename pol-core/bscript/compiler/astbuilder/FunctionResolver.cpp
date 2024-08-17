#include "FunctionResolver.h"

#include <list>

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/Function.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/astbuilder/AvailableParseTree.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/ClassLink.h"
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

void FunctionResolver::force_reference( const ScopeName& scope_name, const SourceLocation& loc )
{
  register_class_link( scope_name, std::make_shared<ClassLink>( loc, scope_name.string() ) );
}

void FunctionResolver::register_available_generated_function( const SourceLocation& loc,
                                                              const ScopableName& name,
                                                              Node* context )
{
  auto apt = AvailableParseTree{ loc, nullptr, name.string(), context };
  register_available_function_parse_tree( loc, name, std::move( apt ) );
}

void FunctionResolver::register_available_user_function(
    const SourceLocation& source_location,
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx, bool force_reference )
{
  ScopableName name( ScopeName::Global, ctx->IDENTIFIER()->getText() );

  // Exported functions are always forced to be referenced so the code generator
  // will emit the function's instructions, even though there is no FunctionCall
  // to it.
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
    EscriptGrammar::EscriptParser::ClassDeclarationContext* ctx,
    Node* top_level_statements_child_node )
{
  register_available_class_decl_parse_tree( loc, ctx, class_name, top_level_statements_child_node );
}

void FunctionResolver::register_class_link( const ScopeName& name,
                                            std::shared_ptr<ClassLink> class_link )
{
  if ( resolve_if_existing( name, class_link ) )
    return;

  unresolved_classes[name].push_back( std::move( class_link ) );
}

void FunctionResolver::register_function_link( const ScopableName& name,
                                               std::shared_ptr<FunctionLink> function_link )
{
  const auto& calling_scope = function_link->calling_scope;
  const auto& unscoped_name = name.name;

  // If a call scope was given, _only_ check that one.
  if ( !name.scope.global() )
  {
    if ( resolve_if_existing( name, function_link ) )
      return;
  }
  else
  {
    // If calling scope present, check, eg. inside Animal class, `foo()` checks
    // `Animal::foo()`.
    if ( !calling_scope.empty() &&
         resolve_if_existing( { calling_scope, unscoped_name }, function_link ) )
      return;

    // Check global scope, only if calling scope is empty. If we are inside eg.
    // `class Animal` and call `foo()`, we do not want to link to an
    // _already-registered_ global function `foo()`. Adding this unresolved
    // function link with a empty scope name will eventually resolve it to the
    // correctly scoped function.
    if ( calling_scope.empty() &&
         resolve_if_existing( ScopableName( ScopeName::Global, unscoped_name ), function_link ) )
      return;
  }

  report.debug( function_link->source_location, "registering funct link {}", name.string() );

  unresolved_function_links[name].push_back( std::move( function_link ) );
}

std::string FunctionResolver::register_function_expression(
    const SourceLocation& source_location,
    EscriptGrammar::EscriptParser::FunctionExpressionContext* ctx )
{
  auto name = function_expression_name( source_location );
  auto apt = AvailableParseTree{ source_location, ctx, "", nullptr };
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
  resolved_functions[{ mf->scope, name }] = mf;
}

void FunctionResolver::register_user_function( const std::string& scope, UserFunction* uf )
{
  ScopableName scoped_name( scope, uf->name );
  auto itr = resolved_functions.find( scoped_name );
  if ( itr != resolved_functions.end() )
  {
    // Throw an exception, this should _never_ happen. If this does, then an
    // AvailableParseTree was visited twice.
    auto msg = fmt::format( "duplicate user function definition for {}: {} vs {}", uf->name,
                            uf->source_location, ( *itr ).second->source_location );
    uf->internal_error( msg );
  }

  resolved_functions[scoped_name] = uf;
  report.debug( uf->source_location, "registering uf {}", scoped_name );

  // Constructors are registered in global scope
  if ( uf->type == UserFunctionType::Constructor )
  {
    ScopableName global_name( ScopeName::Global, uf->name );

    resolved_functions[global_name] = uf;
    report.debug( uf->source_location, "registering uf {}", global_name );
  }
}

void FunctionResolver::register_class_declaration( ClassDeclaration* cd )
{
  resolved_classes[cd->name] = cd;
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

      // Link the function if possible, otherwise try to build it.
      auto link_handled = [&]( const ScopableName& key )
      {
        if ( resolve_if_existing( key, function_link ) )
        {
          // Remove this function link from the list of links.
          function_link_itr = ( *unresolved_itr ).second.erase( function_link_itr );
          return true;
        };

        if ( build_if_available( to_build_ast, calling_scope, key ) )
        {
          // Keep the link in the list, as it may be resolved later.
          ++function_link_itr;
          return true;
        }
        return false;
      };

      report.debug( function_link->source_location, "resolving funct link {}", name );

      // If a call scope was given, _only_ check that one.
      if ( !call_scope.empty() )
      {
        if ( link_handled( { call_scope, unscoped_name } ) )
          continue;
      }
      else
      {
        if ( !calling_scope.empty() )
        {
          std::set<std::string> visited;
          std::list<std::string> to_link( { calling_scope } );
          bool handled = false;

          for ( auto itr = to_link.begin(); !handled && itr != to_link.end(); itr++ )
          {
            auto cd_itr = resolved_classes.find( *itr );
            if ( cd_itr == resolved_classes.end() )
              continue;

            auto cd = cd_itr->second;
            if ( visited.find( cd->name ) != visited.end() )
              continue;

            visited.insert( cd->name );

            if ( link_handled( { cd->name, unscoped_name } ) )
            {
              handled = true;
              break;
            }

            for ( const auto& base_cd_link : cd->base_class_links )
            {
              if ( auto base_cd = base_cd_link->class_declaration() )
              {
                to_link.push_back( base_cd->name );
              }
            }
          }

          if ( handled )
            continue;
        }

        // Check global scope
        if ( link_handled( { ScopeName::Global, unscoped_name } ) )
          continue;
      }

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

  for ( auto unresolved_itr = unresolved_classes.begin();
        unresolved_itr != unresolved_classes.end(); )
  {
    auto scope = unresolved_itr->first;

    for ( auto class_link_itr = ( *unresolved_itr ).second.begin();
          class_link_itr != ( *unresolved_itr ).second.end(); )
    {
      auto& class_link = *class_link_itr;

      report.debug( class_link->source_location, "resolving class link {}", scope.string() );

      // Link the function if possible, otherwise try to build it.
      if ( resolve_if_existing( scope, class_link ) )
      {
        // Remove this function link from the list of links.
        class_link_itr = ( *unresolved_itr ).second.erase( class_link_itr );
      }
      else if ( build_if_available( to_build_ast, scope ) )
      {
        // Keep the link in the list, as it may be resolved later.
        class_link_itr = ( *unresolved_itr ).second.erase( class_link_itr );
      }
      else
      {
        ++class_link_itr;
      }
    }

    // If all links were resolved, remove the entry.
    if ( ( *unresolved_itr ).second.empty() )
    {
      unresolved_itr = unresolved_classes.erase( unresolved_itr );
    }
    else
    {
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

void FunctionResolver::register_available_function_parse_tree(
    const SourceLocation& source_location, const ScopableName& name, const AvailableParseTree& apt )
{
  const auto& unscoped_name = name.name;

  // Eg. "foo" or "Animal::foo"
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
                  scoped_name, previous.source_location );
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
                  scoped_name, what, what, previous->source_location );
  }

  auto itr3 = available_class_decl_parse_trees.find( scope );
  if ( itr3 != available_class_decl_parse_trees.end() )
  {
    auto previous = ( *itr3 ).second;

    report.error( source_location,
                  "User Function '{}' conflicts with Class of the same name.\n"
                  "  Class declaration: {}",
                  scoped_name, previous.source_location );
  }

  available_user_function_parse_trees.insert( { scoped_name, apt } );
}

void FunctionResolver::register_available_user_function_parse_tree(
    const SourceLocation& source_location, antlr4::ParserRuleContext* ctx, const ScopableName& name,
    bool force_reference )
{
  register_available_function_parse_tree( source_location, name,
                                          { source_location, ctx, name.scope.string(), nullptr } );

  if ( force_reference )
  {
    // just make sure there is an entry, so that we build an AST for it
    register_function_link( name, std::make_shared<FunctionLink>(
                                      source_location, name.scope.string() /* calling scope */ ) );
  }
}

void FunctionResolver::register_available_class_decl_parse_tree(
    const SourceLocation& source_location, antlr4::ParserRuleContext* ctx,
    const ScopeName& scope_name, Node* top_level_statements_child_node )
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

  auto apt = AvailableParseTree{ source_location, ctx, name, top_level_statements_child_node };
  available_class_decl_parse_trees.insert( { name, apt } );
}

Function* FunctionResolver::check_existing( const ScopableName& key,
                                            bool requires_constructor ) const
{
  auto itr = resolved_functions.find( key );
  if ( itr != resolved_functions.end() )
  {
    if ( requires_constructor )
    {
      auto uf = dynamic_cast<UserFunction*>( ( *itr ).second );
      if ( !uf || uf->type != UserFunctionType::Constructor )
      {
        return nullptr;
      }
    }

    return ( *itr ).second;
  }
  return nullptr;
}

ClassDeclaration* FunctionResolver::check_existing( const ScopeName& key ) const
{
  auto itr = resolved_classes.find( key );
  if ( itr != resolved_classes.end() )
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
  if ( !call.global() )
  {
    itr = available_user_function_parse_trees.find( call.string() );
    if ( itr != available_user_function_parse_trees.end() )
    {
      to_build_ast.push_back( ( *itr ).second );
      report.debug( ( *itr ).second.source_location, "adding to build funct [call] {}: {}",
                    to_build_ast.back(), call.string() );
      available_user_function_parse_trees.erase( itr );
      return true;
    }

    // Check if there is a class available with that scope, as it could provide this function
    // later.
    if ( build_if_available( to_build_ast, call.scope ) )
    {
      return true;
    }

    // Nothing found. Can't build a scoped function call.
    return false;
  }

  // Inside a scope, eg. `Animal`...
  if ( !calling_scope.empty() )
  {
    // Check if exists in given `scope`, eg. `foo()` checks `Animal::foo()`
    auto handled_by_scope = [&]( const std::string& scope )
    {
      auto scoped_call_name = fmt::format( "{}::{}", scope, call.name );
      itr = available_user_function_parse_trees.find( scoped_call_name );
      if ( itr != available_user_function_parse_trees.end() )
      {
        to_build_ast.push_back( ( *itr ).second );
        report.debug( ( *itr ).second.source_location, "adding to build funct [scoped] {}: {}",
                      to_build_ast.back(), scoped_call_name );
        available_user_function_parse_trees.erase( itr );
        return true;
      }
      return false;
    };

    std::set<std::string> visited;
    std::list<std::string> to_link( { calling_scope } );

    for ( auto itr = to_link.begin(); itr != to_link.end(); itr++ )
    {
      auto cd_itr = resolved_classes.find( *itr );
      if ( cd_itr == resolved_classes.end() )
        continue;

      auto cd = cd_itr->second;
      if ( visited.find( cd->name ) != visited.end() )
        continue;

      visited.insert( cd->name );

      if ( handled_by_scope( cd->name ) )
      {
        itr = to_link.end();
        break;
      }

      for ( const auto& base_cd_link : cd->base_class_links )
      {
        if ( auto base_cd = base_cd_link->class_declaration() )
        {
          to_link.push_back( base_cd->name );
        }
      }
    }
  }

  // Check if there is a class available with the function name.
  if ( build_if_available( to_build_ast, call.name ) )
  {
    return true;
  }

  // If call scope is empty, check if function is a constructor, eg. Foo() -> Foo::Foo
  // The register_user_function will only register the function in global scope if it _is_ a
  // constructor.
  if ( call.scope.global() )
  {
    auto scoped_call_name = fmt::format( "{}::{}", call.name, call.name );
    itr = available_user_function_parse_trees.find( scoped_call_name );
    if ( itr != available_user_function_parse_trees.end() )
    {
      to_build_ast.push_back( ( *itr ).second );
      report.debug( ( *itr ).second.source_location, "adding to build funct [ctor?] {}: {}",
                    to_build_ast.back(), scoped_call_name );
      available_user_function_parse_trees.erase( itr );
      return true;
    }
  }

  // Check if exists in global scope.
  itr = available_user_function_parse_trees.find( call.name );
  if ( itr != available_user_function_parse_trees.end() )
  {
    to_build_ast.push_back( ( *itr ).second );
    report.debug( ( *itr ).second.source_location, "adding to build funct [global] {}: {}",
                  to_build_ast.back(), call.name );
    available_user_function_parse_trees.erase( itr );
    return true;
  }

  return false;
}

bool FunctionResolver::build_if_available( std::vector<AvailableParseTree>& to_build_ast,
                                           const ScopeName& scope )
{
  auto itr = available_class_decl_parse_trees.find( scope.string() );
  if ( itr != available_class_decl_parse_trees.end() )
  {
    to_build_ast.push_back( ( *itr ).second );
    report.debug( ( *itr ).second.source_location, "adding to build class [call.scope] {}: {}",
                  to_build_ast.back(), scope.string() );
    available_class_decl_parse_trees.erase( itr );
    return true;
  }

  return false;
}

bool FunctionResolver::resolve_if_existing( const ScopableName& key,
                                            std::shared_ptr<FunctionLink>& function_link )
{
  auto resolved_function = check_existing( key, function_link->require_ctor );

  if ( resolved_function )
  {
    function_link->link_to( resolved_function );
    report.debug( function_link->source_location, "linking {} to {}::{} @ {}", key,
                  resolved_function->scope, resolved_function->name, (void*)resolved_function );
    return true;
  }

  return false;
}

bool FunctionResolver::resolve_if_existing( const ScopeName& scope,
                                            std::shared_ptr<ClassLink>& class_link )
{
  auto resolved_class = check_existing( scope );

  if ( resolved_class )
  {
    class_link->link_to( resolved_class );
    report.debug( class_link->source_location, "linking class {} to {} @ {}", scope.string(),
                  resolved_class->name, (void*)resolved_class );
    return true;
  }

  return false;
};
}  // namespace Pol::Bscript::Compiler
