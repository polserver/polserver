#include "bscript/compiler/model/ScopeTree.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
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
#include "clib/strutil.h"
#include <algorithm>
#include <memory>
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

UserFunction* ScopeTree::find_user_function( std::string name ) const
{
  Clib::mklowerASCII( name );
  for ( const auto& user_function : workspace.user_functions )
  {
    auto lowered = Clib::strlowerASCII( user_function->name );
    if ( lowered == name )
    {
      return user_function.get();
    }
  }

  return nullptr;
}

std::vector<UserFunction*> ScopeTree::list_user_functions( const ScopeTreeQuery& query,
                                                           const Position& position ) const
{
  std::vector<UserFunction*> results;
  bool can_use_super = false;

  // Check if there is a UserFunction with UserFunctionType::Constructor at the given position that
  // has a base-class links.
  for ( const auto& user_function : workspace.user_functions )
  {
    if ( user_function->type == UserFunctionType::Constructor &&
         user_function->source_location.range.contains( position ) && user_function->class_link )
    {
      if ( auto class_decl = user_function->class_link->class_declaration() )
      {
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

            if ( base_class->constructor_link && base_class->constructor_link->user_function() )
            {
              can_use_super = true;
              break;
            }

            to_visit.insert( to_visit.end(), base_class->base_class_links.begin(),
                             base_class->base_class_links.end() );
          }
        }
      }
      break;
    }
  }


  for ( const auto& user_function : workspace.user_functions )
  {
    if ( user_function->type != UserFunctionType::Expression &&
         ( user_function->type != UserFunctionType::Super || can_use_super ) &&
         starts_with( user_function->name, query.prefix ) &&
         ( Clib::caseInsensitiveEqual( query.prefix_scope.string(), user_function->scope ) ||
           Clib::caseInsensitiveEqual( query.calling_scope, user_function->scope ) ) )
    {
      results.push_back( user_function.get() );
    }
  }

  return results;
}

ModuleFunctionDeclaration* ScopeTree::find_module_function( std::string name ) const
{
  Clib::mklowerASCII( name );

  for ( const auto& module_function : workspace.module_function_declarations )
  {
    auto lowered = Clib::strlowerASCII( module_function->name );

    if ( lowered == name )
    {
      return module_function.get();
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

std::vector<ConstDeclaration*> ScopeTree::list_constants( const ScopeTreeQuery& query ) const
{
  // Constants are not scoped, so if a non-empty scope is given, return empty list.
  if ( !query.prefix_scope.global() )
    return {};

  return workspace.constants.list( query.prefix );
}

std::vector<ClassDeclaration*> ScopeTree::list_classes( const ScopeTreeQuery& query ) const
{
  // Classes are not scoped, so if a non-empty scope is given, return empty list.
  if ( !query.prefix_scope.global() )
    return {};

  std::vector<ClassDeclaration*> results;

  for ( const auto& class_decl : workspace.class_declarations )
  {
    if ( starts_with( class_decl->name, query.prefix ) )
    {
      results.push_back( class_decl.get() );
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
    // bool starts_with =
    //     std::equal( module_path.begin(), module_path.end(), ident->pathname.begin() );


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
