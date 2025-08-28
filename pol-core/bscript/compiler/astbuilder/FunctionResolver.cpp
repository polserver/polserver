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
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
FunctionResolver::FunctionResolver( CompilerWorkspace& workspace, Report& report )
    : workspace( workspace ), report( report )
{
}
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
                                                              Node* context, UserFunctionType type )
{
  auto agf = std::make_unique<AvailableGeneratedFunction>( loc, context, name, type );
  register_available_function_parse_tree( loc, name, std::move( agf ) );
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
  auto apt =
      std::make_unique<AvailableParseTree>( source_location, ctx, ScopeName::Global, nullptr );
  available_user_function_parse_trees[name] = std::move( apt );
  return name;
}

void FunctionResolver::register_module_function( ModuleFunctionDeclaration* mf )
{
  const auto& name = mf->name;

  auto itr = available_user_function_parse_trees.find( name );
  if ( itr != available_user_function_parse_trees.end() )
  {
    const auto& previous = ( *itr ).second;

    report.error( previous->source_location,
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

  // Since we are _registering_ the class declaration, that means either the
  // user has actively instantiated the class via `Foo()` or a base class has
  // requested `Foo`. Register all the class' methods and include them at
  // instruction generation. See also: `Optimizer::optimize` where Methods and
  // Constructors are force-referenced.
  //
  // The `methods` object only contains methods, as the constructor is in the
  // `constructor_link`.
  for ( const auto& [method_name, function_link] : cd->methods )
  {
    register_function_link( { cd->name, method_name }, function_link );
  }

  // Keep track of class children for constructor generation.
  for ( const auto& link : cd->base_class_links )
  {
    class_children[link->name].push_back( cd );
  }

  // If this class declaration has no constructor, check base classes that may already be resolved.
  if ( !cd->constructor_link || !cd->has_super_ctor )
  {
    std::set<ClassDeclaration*> visited;
    std::list<std::shared_ptr<ClassLink>> to_visit;

    to_visit.insert( to_visit.end(), cd->base_class_links.begin(), cd->base_class_links.end() );
    for ( const auto& link : to_visit )
    {
      if ( auto base_cd = link->class_declaration() )
      {
        if ( visited.find( base_cd ) != visited.end() )
        {
          continue;
        }
        visited.insert( base_cd );

        if ( base_cd->constructor_link )
        {
          if ( !cd->constructor_link )
          {
            ScopableName ctor_name( cd->name, cd->name );
            cd->constructor_link = std::make_unique<FunctionLink>( cd->source_location, cd->name,
                                                                   true /* requires_ctor */ );

            register_function_link( ctor_name, cd->constructor_link );

            register_available_generated_function( cd->source_location, ctor_name, cd,
                                                   UserFunctionType::Constructor );
          }

          if ( !cd->has_super_ctor )
          {
            ScopableName child_super( cd->name, "super" );
            cd->has_super_ctor = true;

            register_available_generated_function( cd->source_location, child_super, cd,
                                                   UserFunctionType::Super );
          }

          break;
        }

        to_visit.insert( to_visit.end(), base_cd->base_class_links.begin(),
                         base_cd->base_class_links.end() );
      }
    }
  }

  // If this class declaration has a constructor, and there are classes that inherit
  // from it, generate a constructor for my descendants that do not have one.
  if ( cd->constructor_link && class_children.contains( cd->name ) )
  {
    std::set<ClassDeclaration*> visited;
    std::list<ClassDeclaration*> to_visit;

    to_visit.insert( to_visit.end(), class_children[cd->name].begin(),
                     class_children[cd->name].end() );

    for ( const auto& child_cd : to_visit )
    {
      if ( visited.contains( child_cd ) )
        continue;

      visited.insert( child_cd );

      if ( !child_cd->has_super_ctor )
      {
        ScopableName child_super( child_cd->name, "super" );
        child_cd->has_super_ctor = true;

        register_available_generated_function( child_cd->source_location, child_super, child_cd,
                                               UserFunctionType::Super );
      }

      if ( !child_cd->constructor_link )
      {
        ScopableName child_ctor( child_cd->name, child_cd->name );
        child_cd->constructor_link = std::make_unique<FunctionLink>(
            child_cd->source_location, child_cd->name, true /* requires_ctor */ );

        register_function_link( child_ctor, child_cd->constructor_link );

        register_available_generated_function( cd->source_location, child_ctor, child_cd,
                                               UserFunctionType::Constructor );
      }

      to_visit.insert( to_visit.end(), class_children[child_cd->name].begin(),
                       class_children[child_cd->name].end() );
    }
  }
}

bool FunctionResolver::resolve(
    std::vector<std::unique_ptr<AvailableSecondPassTarget>>& to_build_ast )
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
      bool is_super_scoped = call_scope.super();

      // Link the function if possible, otherwise try to build it.
      auto link_handled = [&]( const ScopableName& key )
      {
        // A super:: call cannot be handled by the current call scope.
        if ( call_scope.super() && Clib::caseInsensitiveEqual( key.scope.string(), calling_scope ) )
        {
          return false;
        }

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

      auto handled_by_scopes = [&]( const std::string& starting_scope )
      {
        std::set<std::string> visited;
        std::list<std::string> to_check( { starting_scope } );
        bool handled = false;

        for ( auto to_check_itr = to_check.begin(); !handled && to_check_itr != to_check.end();
              to_check_itr = to_check.erase( to_check_itr ) )
        {
          if ( visited.find( *to_check_itr ) != visited.end() )
            continue;

          visited.insert( *to_check_itr );

          if ( link_handled( { *to_check_itr, unscoped_name } ) )
          {
            return true;
          }

          auto cd_itr = resolved_classes.find( *to_check_itr );
          if ( cd_itr == resolved_classes.end() )
            continue;

          auto cd = cd_itr->second;

          for ( const auto& base_cd_link : cd->base_class_links )
          {
            if ( auto base_cd = base_cd_link->class_declaration() )
            {
              to_check.push_back( base_cd->name );
            }
            // If using a call to super:: and this base class has not been
            // loaded, we can't resolve the link just yet: we wait until the
            // base class list (at least, for this current `cd`) is loaded.
            else if ( is_super_scoped )
            {
              return false;
            }
          }
        }

        return false;
      };

      report.debug( function_link->source_location, "resolving funct link {}", name );

      // If a call scope was given (except super::), we check that call scope
      // and its ancestors _without_ checking Global (ie. skipping the check
      // done in the `else` block)
      if ( !call_scope.empty() && !call_scope.super() )
      {
        if ( handled_by_scopes( call_scope.string() ) )
          continue;
      }
      else
      {
        if ( !calling_scope.empty() && handled_by_scopes( calling_scope ) )
          continue;

        // Check global scope (if not explicitly calling super:: scope)
        if ( !call_scope.super() && link_handled( { ScopeName::Global, unscoped_name } ) )
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

  // If any class link was resolved, we need to trigger
  // `CompilerWorkspaceBuilder::build_referenced_user_functions` to run the
  // resolution loop again.
  bool any_classes_linked = false;

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
        any_classes_linked = true;
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

  return any_classes_linked || !to_build_ast.empty();
}

std::string FunctionResolver::function_expression_name( const SourceLocation& source_location )
{
  return fmt::format( "funcexpr@{}:{}:{}", source_location.source_file_identifier->index,
                      source_location.range.start.line_number,
                      source_location.range.start.character_column );
}

void FunctionResolver::register_available_function_parse_tree(
    const SourceLocation& source_location, const ScopableName& name,
    std::unique_ptr<AvailableSecondPassTarget> apt )
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
    auto& previous = ( *itr ).second;

    report.error( source_location,
                  "Function '{}' defined more than once.\n"
                  "  Previous declaration: {}",
                  scoped_name, previous->source_location );
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
    auto& previous = ( *itr3 ).second;

    report.error( source_location,
                  "User Function '{}' conflicts with Class of the same name.\n"
                  "  Class declaration: {}",
                  scoped_name, previous->source_location );
  }

  available_user_function_parse_trees[scoped_name] = std::move( apt );
}

void FunctionResolver::register_available_user_function_parse_tree(
    const SourceLocation& source_location, antlr4::ParserRuleContext* ctx, const ScopableName& name,
    bool force_reference )
{
  register_available_function_parse_tree(
      source_location, name,
      std::make_unique<AvailableParseTree>( source_location, ctx, name.scope, nullptr ) );

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
    auto& previous = ( *itr ).second;

    report.error( source_location,
                  "Class '{}' conflicts with User Function of the same name.\n"
                  "  Previous declaration: {}",
                  name, previous->source_location );
  }

  if ( auto itr2 = workspace.all_class_locations.find( name );
       itr2 != workspace.all_class_locations.end() )
  {
    auto& previous = ( *itr2 ).second;
    report.error( source_location,
                  "Class '{}' defined more than once.\n"
                  "  Previous declaration: {}",
                  name, previous );
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

  auto apt = std::make_unique<AvailableParseTree>( source_location, ctx, scope_name,
                                                   top_level_statements_child_node );
  available_class_decl_parse_trees[name] = std::move( apt );
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

bool FunctionResolver::build_if_available(
    std::vector<std::unique_ptr<AvailableSecondPassTarget>>& to_build_ast,
    const std::string& calling_scope, const ScopableName& call )
{
  AvailableParseTreeMap::iterator itr;

  // If a call scope was given, _only_ check that one (except if super:: provided)
  // eg. `Animal::foo()` will only search for `Animal::foo()`, disregarding a possible parent
  // scoped `::foo()`.
  if ( !call.global() && !call.scope.super() )
  {
    itr = available_user_function_parse_trees.find( call.string() );
    if ( itr != available_user_function_parse_trees.end() )
    {
      to_build_ast.push_back( std::move( ( *itr ).second ) );
      available_user_function_parse_trees.erase( itr );
      report.debug( to_build_ast.back()->source_location, "adding to build funct [call] {}: {}",
                    *to_build_ast.back(), call.string() );
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
      // Skip checking current scope if doing `super::` call.
      if ( call.scope.super() && Clib::caseInsensitiveEqual( scope, calling_scope ) )
      {
        return false;
      }
      auto scoped_call_name = fmt::format( "{}::{}", scope, call.name );
      itr = available_user_function_parse_trees.find( scoped_call_name );
      if ( itr != available_user_function_parse_trees.end() )
      {
        to_build_ast.push_back( std::move( ( *itr ).second ) );
        available_user_function_parse_trees.erase( itr );
        report.debug( to_build_ast.back()->source_location, "adding to build funct [scoped] {}: {}",
                      *to_build_ast.back(), scoped_call_name );
        return true;
      }
      return false;
    };

    std::set<std::string> visited;
    std::list<std::string> to_check( { calling_scope } );

    for ( auto to_check_itr = to_check.begin(); to_check_itr != to_check.end();
          to_check_itr = to_check.erase( to_check_itr ) )
    {
      auto cd_itr = resolved_classes.find( *to_check_itr );
      if ( cd_itr == resolved_classes.end() )
        continue;

      auto cd = cd_itr->second;
      if ( visited.find( cd->name ) != visited.end() )
        continue;

      visited.insert( cd->name );

      if ( handled_by_scope( cd->name ) )
      {
        break;
      }

      for ( const auto& base_cd_link : cd->base_class_links )
      {
        if ( auto base_cd = base_cd_link->class_declaration() )
        {
          to_check.push_back( base_cd->name );
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
      to_build_ast.push_back( std::move( ( *itr ).second ) );
      available_user_function_parse_trees.erase( itr );
      report.debug( to_build_ast.back()->source_location, "adding to build funct [ctor?] {}: {}",
                    *to_build_ast.back(), scoped_call_name );
      return true;
    }
  }

  // Check if exists in global scope.
  itr = available_user_function_parse_trees.find( call.name );
  if ( itr != available_user_function_parse_trees.end() )
  {
    to_build_ast.push_back( std::move( ( *itr ).second ) );
    available_user_function_parse_trees.erase( itr );
    report.debug( to_build_ast.back()->source_location, "adding to build funct [global] {}: {}",
                  *to_build_ast.back(), call.name );
    return true;
  }

  return false;
}

bool FunctionResolver::build_if_available(
    std::vector<std::unique_ptr<AvailableSecondPassTarget>>& to_build_ast, const ScopeName& scope )
{
  auto itr = available_class_decl_parse_trees.find( scope.string() );
  if ( itr != available_class_decl_parse_trees.end() )
  {
    to_build_ast.push_back( std::move( ( *itr ).second ) );
    available_class_decl_parse_trees.erase( itr );
    report.debug( to_build_ast.back()->source_location, "adding to build class [call.scope] {}: {}",
                  *to_build_ast.back(), scope.string() );
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
