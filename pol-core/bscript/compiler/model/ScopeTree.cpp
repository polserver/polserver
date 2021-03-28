#include "bscript/compiler/model/ScopeTree.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/Variable.h"
#include "clib/logfacility.h"
#include <memory>

namespace Pol::Bscript::Compiler
{
ScopeInfo::ScopeInfo( const SourceLocation& loc ) : location( loc ) {}

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

std::shared_ptr<Variable> ScopeTree::find_variable( const std::string& name,
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

std::shared_ptr<Variable> ScopeInfo::resolve( const std::string& name ) const
{
  auto itr = variables.find( name );
  if ( itr != variables.end() )
  {
    return itr->second;
  }

  return {};
}

std::shared_ptr<Variable> ScopeInfo::walk( const std::string& name, const Position& position ) const
{
  auto variable = std::shared_ptr<Variable>();

  if ( !location.contains( position ) )
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
      if ( child->location.contains( position ) )
      {
        current = child.get();
        found = true;
        break;
      }
    }
  } while ( found );

  return variable;
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
