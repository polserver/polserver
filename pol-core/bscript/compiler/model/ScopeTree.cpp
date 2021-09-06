#include "bscript/compiler/model/ScopeTree.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/model/Variable.h"
#include "clib/logfacility.h"
#include "clib/strutil.h"
#include <algorithm>
#include <memory>

namespace Pol::Bscript::Compiler
{
ScopeInfo::ScopeInfo( const SourceLocation& loc ) : location( loc ) {}

ScopeTree::ScopeTree( CompilerWorkspace& workspace ) : workspace( workspace ) {}

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

std::vector<UserFunction*> ScopeTree::list_user_functions( std::string name ) const
{
  std::vector<UserFunction*> results;
  Clib::mklowerASCII( name );
  for ( const auto& user_function : workspace.user_functions )
  {
    auto lowered = Clib::strlowerASCII( user_function->name );
    if ( lowered.rfind( name, 0 ) == 0 )
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

std::vector<ModuleFunctionDeclaration*> ScopeTree::list_module_functions( std::string name ) const
{
  std::vector<ModuleFunctionDeclaration*> results;
  Clib::mklowerASCII( name );
  for ( const auto& module_function : workspace.module_function_declarations )
  {
    auto lowered = Clib::strlowerASCII( module_function->name );

    if ( lowered.rfind( name, 0 ) == 0 )
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

std::vector<std::shared_ptr<Variable>> ScopeTree::list_variables( std::string prefix,
                                                                  const Position& position ) const
{
  Clib::mklowerASCII( prefix );
  auto variables = scopes[0]->walk_list( prefix, position );

  std::for_each( globals.begin(), globals.end(),
                 [&]( const auto& p )
                 {
                   auto lowered = Clib::strlowerASCII( p.first );
                   if ( lowered.rfind( prefix, 0 ) == 0 )
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

std::vector<ConstDeclaration*> ScopeTree::list_constants( std::string name ) const
{
  return workspace.constants.list( name );
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
