#include "bscript/compiler/model/ScopeTree.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/Variable.h"
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

void ScopeInfo::describe_tree_to_indented( fmt::Writer& w, const ScopeInfo& node, unsigned indent )
{
  w << std::string( indent * 2, ' ' ) << "- ";

  for ( const auto& variable : node.variables )
  {
    w << variable.first << " ";
  }
  w << "\n";
  for ( const auto& child : node.children )
  {
    if ( child )
      describe_tree_to_indented( w, *child, indent + 1 );
  }
}

fmt::Writer& operator<<( fmt::Writer& w, const ScopeInfo& node )
{
  node.describe_tree_to_indented( w, node, 0 );
  return w;
}

fmt::Writer& operator<<( fmt::Writer& w, const ScopeTree& node )
{
  for ( const auto& child : node.scopes )
  {
    if ( child )
      w << *child;
  }
  return w;
}

}  // namespace Pol::Bscript::Compiler
