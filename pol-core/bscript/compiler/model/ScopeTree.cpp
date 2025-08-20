#include "bscript/compiler/model/ScopeTree.h"
#include "ScopeTree.h"
#include "bscript/compiler/analyzer/ThisMemberAssignmentGatherer.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/MemberAssignment.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/ClassLink.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/model/Variable.h"
#include "bscript/compilercfg.h"
#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/maputil.h"
#include "clib/strutil.h"
#include <algorithm>
#include <memory>
#include <set>
#include <utility>

namespace Pol::Bscript::Compiler
{
ScopeInfo::ScopeInfo( const SourceLocation& loc ) : location( loc ) {}

ScopeTree::ScopeTree( CompilerWorkspace& workspace ) : workspace( workspace ) {}

bool starts_with( const std::string& str, const std::string& prefix )
{
  if ( prefix.size() > str.size() )
  {
    return false;
  }

  // Perform case-insensitive comparison
  return std::equal( prefix.begin(), prefix.end(), str.begin(),
                     []( char a, char b ) { return std::tolower( a ) == std::tolower( b ); } );
}

void ScopeTree::push_scope( const SourceLocation& location )
{
  if ( location.source_file_identifier->index > 0 )
  {
    ignored++;
    return;
  }

  auto scope_info = std::make_shared<ScopeInfo>( location );
  if ( !scopes.empty() )
  {
    scopes.back()->children.push_back( scope_info );
  }
  scopes.push_back( scope_info );
}

void ScopeTree::pop_scope( std::vector<std::shared_ptr<Variable>> variables )
{
  if ( ignored > 0 )
  {
    ignored--;
    return;
  }
  auto scope_info = scopes.back();
  scopes.pop_back();
  for ( const auto& variable : variables )
  {
    scope_info->variables.emplace( variable.get()->name, variable );
  }
}

void ScopeTree::set_globals( std::vector<std::shared_ptr<Variable>> variables )
{
  for ( const auto& variable : variables )
  {
    globals.emplace( variable.get()->name, variable );
  }
}

// Callable returns true if the search should stop.
template <typename Callable>
void with_base_classes( ClassDeclaration* class_decl, Callable callable )
{
  if ( !class_decl )
    return;

  std::list<std::shared_ptr<ClassLink>> to_visit( class_decl->base_class_links.begin(),
                                                  class_decl->base_class_links.end() );
  std::set<ClassDeclaration*> visited;
  while ( !to_visit.empty() )
  {
    auto base_class_link = to_visit.front();
    to_visit.pop_front();

    if ( auto base_class = base_class_link->class_declaration() )
    {
      if ( visited.find( base_class ) != visited.end() )
      {
        continue;
      }

      visited.insert( base_class );

      if ( callable( base_class ) )
      {
        return;
      }

      to_visit.insert( to_visit.end(), base_class->base_class_links.begin(),
                       base_class->base_class_links.end() );
    }
  }
}

UserFunction* ScopeTree::find_user_function( const ScopeTreeQuery& query ) const
{
  if ( query.prefix_scope.super() && !query.calling_scope.empty() )
  {
    auto class_decl = find_class( query.calling_scope );
    UserFunction* user_function = nullptr;

    with_base_classes(
        class_decl,
        [&]( ClassDeclaration* base_class ) -> bool
        {
          auto user_function_itr = std::find_if(
              workspace.user_functions.begin(), workspace.user_functions.end(),
              [&]( const auto& user_function )
              {
                return Clib::caseInsensitiveEqual( user_function->name, query.prefix ) &&
                       Clib::caseInsensitiveEqual( user_function->scope, base_class->name );
              } );

          if ( user_function_itr != workspace.user_functions.end() )
          {
            user_function = user_function_itr->get();
            return true;
          }
          return false;
        } );

    return user_function;
  }
  else
  {
    for ( const auto& user_function : workspace.user_functions )
    {
      if ( !Clib::caseInsensitiveEqual( user_function->name, query.prefix ) )
        continue;

      if ( query.prefix_scope.empty() )
      {
        if ( Clib::caseInsensitiveEqual( user_function->scope, query.calling_scope ) ||
             user_function->scope.empty() ||
             ( ( query.calling_scope.empty() ||
                 user_function->type == UserFunctionType::Expression ) &&
               user_function->scope.empty() ) )
        {
          return user_function.get();
        }
      }
      else
      {
        if ( ( query.prefix_scope.global() && user_function->scope.empty() ) ||
             Clib::caseInsensitiveEqual( query.prefix_scope.string(), user_function->scope ) )
        {
          return user_function.get();
        }
      }
    }
  }

  return nullptr;
}

std::vector<UserFunction*> ScopeTree::list_user_functions( const ScopeTreeQuery& query,
                                                           const Position& position ) const
{
  std::vector<UserFunction*> results;

  if ( query.prefix_scope.super() )
  {
    if ( !query.calling_scope.empty() && !query.current_user_function.empty() )
    {
      auto class_decl = find_class( query.calling_scope );

      with_base_classes(
          class_decl,
          [&]( ClassDeclaration* base_class ) -> bool
          {
            for ( const auto& user_function : workspace.user_functions )
            {
              if ( starts_with( user_function->name, query.prefix ) &&
                   Clib::caseInsensitiveEqual( user_function->scope, base_class->name ) )
              {
                results.push_back( user_function.get() );
              }
            }
            return false;
          } );
    }
  }
  else
  {
    bool can_use_super = false;

    // Check if there is a UserFunction with UserFunctionType::Constructor at the given position
    // that has a base-class links.
    for ( const auto& user_function : workspace.user_functions )
    {
      if ( user_function->type == UserFunctionType::Constructor &&
           user_function->source_location.range.contains( position ) && user_function->class_link )
      {
        with_base_classes(
            user_function->class_link->class_declaration(),
            [&]( ClassDeclaration* base_class )
            {
              if ( base_class->constructor_link && base_class->constructor_link->user_function() )
              {
                can_use_super = true;
                return true;
              }
              return false;
            } );
        break;
      }
    }


    for ( const auto& user_function : workspace.user_functions )
    {
      if ( user_function->type != UserFunctionType::Expression &&
           ( user_function->type != UserFunctionType::Super || can_use_super ) &&
           starts_with( user_function->name, query.prefix ) &&
           ( ( !query.prefix_scope.empty() &&
               Clib::caseInsensitiveEqual( query.prefix_scope.string(), user_function->scope ) ) ||
             ( query.prefix_scope.empty() &&
               ( Clib::caseInsensitiveEqual( query.calling_scope, user_function->scope ) ||
                 user_function->scope.empty() ) ) ) )
      {
        results.push_back( user_function.get() );
      }
    }
  }

  return results;
}

ModuleFunctionDeclaration* ScopeTree::find_module_function( const ScopeTreeQuery& query ) const
{
  for ( const auto& module_function : workspace.module_function_declarations )
  {
    if ( query.prefix_scope.empty() )
    {
      if ( Clib::caseInsensitiveEqual( module_function->name, query.prefix ) )
      {
        return module_function.get();
      }
    }
    else
    {
      if ( Clib::caseInsensitiveEqual( module_function->name, query.prefix ) &&
           ( query.prefix_scope.global() ||
             Clib::caseInsensitiveEqual( query.prefix_scope.string(), module_function->scope ) ) )
      {
        return module_function.get();
      }
    }
  }

  return nullptr;
}

std::vector<ModuleFunctionDeclaration*> ScopeTree::list_module_functions(
    const ScopeTreeQuery& query ) const
{
  std::vector<ModuleFunctionDeclaration*> results;
  for ( const auto& module_function : workspace.module_function_declarations )
  {
    if ( starts_with( module_function->name, query.prefix ) &&
         ( query.prefix_scope.global() ||
           starts_with( module_function->scope, query.prefix_scope.string() ) ) )
    {
      results.push_back( module_function.get() );
    }
  }

  return results;
}

std::shared_ptr<Variable> ScopeTree::find_variable( std::string name,
                                                    const Position& position ) const
{
  auto variable = scopes[0]->walk( name, position );
  if ( variable )
    return variable;

  auto itr = globals.find( name );
  if ( itr != globals.end() )
  {
    return itr->second;
  }

  return {};
}

std::vector<std::shared_ptr<Variable>> ScopeTree::list_variables( const ScopeTreeQuery& query,
                                                                  const Position& position ) const
{
  auto prefix = Clib::strlowerASCII( query.prefix );

  // Can only get local variables if there is no prefix_scope, since variables
  // with a prefix_scope are global.
  auto variables = query.prefix_scope.empty() ? scopes[0]->walk_list( prefix, position )
                                              : std::vector<std::shared_ptr<Variable>>();

  // TODO BUG: Global variables are returned outside of the `position` context.
  std::for_each(
      globals.begin(), globals.end(),
      [&]( const auto& p )
      {
        auto scoped_name = p.second->scoped_name();
        if ( Clib::caseInsensitiveEqual( query.prefix_scope.string(),
                                         scoped_name.scope.string() ) ||
             ( !query.calling_scope.empty() &&
               Clib::caseInsensitiveEqual( query.calling_scope, scoped_name.scope.string() ) ) )
        {
          variables.push_back( p.second );
        }
      } );
  return variables;
}

ConstDeclaration* ScopeTree::find_constant( std::string name ) const
{
  return workspace.constants.find( name );
}

ClassDeclaration* ScopeTree::find_class( const std::string& name ) const
{
  auto class_itr =
      std::find_if( workspace.class_declarations.begin(), workspace.class_declarations.end(),
                    [&]( const auto& class_decl )
                    { return Clib::caseInsensitiveEqual( class_decl->name, name ); } );

  if ( class_itr != workspace.class_declarations.end() )
  {
    return class_itr->get();
  }
  return nullptr;
}

MemberAssignment* ScopeTree::find_class_member( const ScopeTreeQuery& query ) const
{
  MemberAssignment* result = nullptr;

  if ( query.calling_scope.empty() )
    return result;

  auto class_decl = find_class( query.calling_scope );
  if ( !class_decl )
    return result;

  auto check_class = [&]( ClassDeclaration* cd ) -> bool
  {
    if ( auto constructor_link = cd->constructor_link )
    {
      if ( auto user_function = constructor_link->user_function() )
      {
        ThisMemberAssignmentGatherer gatherer;
        user_function->accept( gatherer );
        for ( auto* assignment : gatherer.assignments )
        {
          if ( Clib::caseInsensitiveEqual( assignment->name, query.prefix ) )
          {
            result = assignment;
            return true;
          }
        }
      }
    }
    return false;
  };

  if ( !check_class( class_decl ) )
    with_base_classes( class_decl, check_class );

  return result;
}

UserFunction* ScopeTree::find_class_method( const ScopeTreeQuery& query ) const
{
  UserFunction* result = nullptr;

  if ( query.calling_scope.empty() )
    return result;

  auto class_decl = find_class( query.calling_scope );
  if ( !class_decl )
    return result;

  auto check_class = [&]( ClassDeclaration* cd ) -> bool
  {
    for ( auto& [_, function_link] : cd->methods )
    {
      if ( auto user_function = function_link->user_function() )
      {
        if ( Clib::caseInsensitiveEqual( user_function->name, query.prefix ) )
        {
          result = user_function;
          return true;
        }
      }
    }
    return false;
  };

  if ( !check_class( class_decl ) )
    with_base_classes( class_decl, check_class );

  return result;
}

std::vector<ConstDeclaration*> ScopeTree::list_constants( const ScopeTreeQuery& query ) const
{
  if ( !query.prefix_scope.global() )
    return workspace.constants.list(
        fmt::format( "{}::{}", query.prefix_scope.string(), query.prefix ) );

  // If in a calling scope, we need to return constants for both the calling
  // scope and global scope
  if ( !query.calling_scope.empty() )
  {
    std::vector<ConstDeclaration*> results;
    auto calling_scope_constants =
        workspace.constants.list( fmt::format( "{}::{}", query.calling_scope, query.prefix ) );

    std::move( calling_scope_constants.begin(), calling_scope_constants.end(),
               std::back_inserter( results ) );

    auto prefix_only_constants = workspace.constants.list( query.prefix );

    std::move( prefix_only_constants.begin(), prefix_only_constants.end(),
               std::back_inserter( results ) );

    return results;
  }
  else
  {
    return workspace.constants.list( query.prefix );
  }
}

std::vector<std::string> ScopeTree::list_scopes( const ScopeTreeQuery& query ) const
{
  // Classes are not scoped, so if a non-empty scope is given, return empty list.
  if ( !query.prefix_scope.global() )
    return {};

  std::vector<std::string> results;

  for ( const auto& class_decl : workspace.class_declarations )
  {
    if ( starts_with( class_decl->name, query.prefix ) )
    {
      results.push_back( class_decl->name );
    }
  }

  // Add `super` if inside a scoped user function that has at least one base class that defines a
  // method.
  if ( query.prefix_scope.empty() && !query.current_user_function.empty() &&
       !query.calling_scope.empty() )
  {
    auto class_decl = find_class( query.calling_scope );
    bool add_super = false;

    with_base_classes( class_decl,
                       [&]( ClassDeclaration* base_class ) -> bool
                       {
                         auto user_function_itr = std::find_if(
                             workspace.user_functions.begin(), workspace.user_functions.end(),
                             [&]( const auto& user_function ) {
                               return Pol::Clib::caseInsensitiveEqual( user_function->scope,
                                                                       base_class->name );
                             } );

                         if ( user_function_itr != workspace.user_functions.end() )
                         {
                           add_super = true;
                           return true;
                         }
                         return false;
                       } );

    if ( add_super )
    {
      results.push_back( "super" );
    }
  }

  return results;
}

std::vector<std::string> ScopeTree::list_modules( const ScopeTreeQuery& query ) const
{
  std::vector<std::string> results;
  auto module_path = Clib::FullPath( compilercfg.ModuleDirectory.c_str() );

  // Modules are not scoped, so if a non-empty scope is given, return empty list.
  if ( !query.prefix_scope.global() )
    return {};

  for ( const auto& ident : workspace.referenced_source_file_identifiers )
  {
    if ( starts_with( ident->pathname, module_path ) )
    {
      // Check if ident->pathname ends with ".em":
      bool ends_with = ident->pathname.size() >= 3 &&
                       Clib::caseInsensitiveEqual(
                           std::string( ident->pathname.end() - 3, ident->pathname.end() ), ".em" );

      if ( ends_with )
      {
        auto module_name = Clib::GetFilePart( ident->pathname.c_str() );

        // Check that `module_name` starts with `name`
        if ( starts_with( module_name, query.prefix ) )
        {
          // Remove the ".em" extension from module_name
          results.push_back( module_name.substr( 0, module_name.size() - 3 ) );
        }
      }
    }
  }

  return results;
}

// Only works for `this.foo` and not `this["foo"]`
std::vector<MemberAssignment*> ScopeTree::list_class_members( const ScopeTreeQuery& query ) const
{
  std::vector<MemberAssignment*> results;
  std::set<std::string, Clib::ci_cmp_pred> added_names;

  if ( query.calling_scope.empty() )
    return results;

  auto class_decl = find_class( query.calling_scope );
  if ( !class_decl )
    return results;

  auto check_class = [&]( ClassDeclaration* cd ) -> bool
  {
    if ( auto constructor_link = cd->constructor_link )
    {
      if ( auto user_function = constructor_link->user_function() )
      {
        ThisMemberAssignmentGatherer gatherer;
        user_function->accept( gatherer );
        for ( auto* assignment : gatherer.assignments )
        {
          if ( starts_with( assignment->name, query.prefix ) &&
               added_names.find( assignment->name ) == added_names.end() )
          {
            results.push_back( assignment );
            added_names.emplace( assignment->name );
          }
        }
      }
    }
    return false;
  };

  check_class( class_decl );
  with_base_classes( class_decl, check_class );
  return results;
}

std::vector<UserFunction*> ScopeTree::list_class_methods( const ScopeTreeQuery& query ) const
{
  std::vector<UserFunction*> results;
  std::set<std::string, Clib::ci_cmp_pred> added_names;

  if ( query.calling_scope.empty() )
    return results;

  auto class_decl = find_class( query.calling_scope );
  if ( !class_decl )
    return results;

  auto check_class = [&]( ClassDeclaration* cd ) -> bool
  {
    for ( auto& [_, function_link] : cd->methods )
    {
      if ( auto user_function = function_link->user_function() )
      {
        if ( starts_with( user_function->name, query.prefix ) &&
             added_names.find( user_function->name ) == added_names.end() )
        {
          results.push_back( user_function );
          added_names.emplace( user_function->name );
        }
      }
    }
    return false;
  };

  check_class( class_decl );
  with_base_classes( class_decl, check_class );

  return results;
}

std::shared_ptr<Variable> ScopeInfo::resolve( const std::string& name ) const
{
  auto itr = variables.find( name );
  if ( itr != variables.end() )
  {
    return itr->second;
  }

  return {};
}

std::vector<std::shared_ptr<Variable>> ScopeInfo::resolve_list( const std::string& prefix ) const
{
  std::vector<std::shared_ptr<Variable>> results;
  std::for_each( variables.begin(), variables.end(),
                 [&]( const auto& p )
                 {
                   if ( p.first.rfind( prefix, 0 ) == 0 )
                   {
                     results.push_back( p.second );
                   }
                 } );

  return results;
}

std::shared_ptr<Variable> ScopeInfo::walk( const std::string& name, const Position& position ) const
{
  auto variable = std::shared_ptr<Variable>();

  if ( !location.range.contains( position ) )
  {
    return variable;
  }

  const ScopeInfo* current = this;

  bool found;
  do
  {
    found = false;

    auto current_symbol = current->resolve( name );
    if ( current_symbol )
    {
      variable = current_symbol;
    }

    for ( const auto& child : current->children )
    {
      if ( child->location.range.contains( position ) )
      {
        current = child.get();
        found = true;
        break;
      }
    }
  } while ( found );

  return variable;
}

std::vector<std::shared_ptr<Variable>> ScopeInfo::walk_list( const std::string& name,
                                                             const Position& position ) const
{
  std::vector<std::shared_ptr<Variable>> results;

  if ( !location.range.contains( position ) )
  {
    return results;
  }

  const ScopeInfo* current = this;

  bool found;
  do
  {
    found = false;

    auto current_symbol = current->resolve_list( name );
    if ( !current_symbol.empty() )
    {
      results.insert( results.end(), std::make_move_iterator( current_symbol.begin() ),
                      std::make_move_iterator( current_symbol.end() ) );
      current_symbol.clear();
    }

    for ( const auto& child : current->children )
    {
      if ( child->location.range.contains( position ) )
      {
        current = child.get();
        found = true;
        break;
      }
    }
  } while ( found );

  return results;
}

void ScopeInfo::describe_tree_to_indented( std::string& w, const ScopeInfo& node, unsigned indent )
{
  w += std::string( indent * 2, ' ' ) + "- ";

  for ( const auto& variable : node.variables )
  {
    w += variable.first + " ";
  }
  w += "\n";
  for ( const auto& child : node.children )
  {
    if ( child )
      describe_tree_to_indented( w, *child, indent + 1 );
  }
}
}  // namespace Pol::Bscript::Compiler
