#include "Node.h"


namespace Pol::Bscript::Compiler
{
Node::Node( const SourceLocation& source_location, NodeVector children )
    : children( std::move( children ) ), source_location( source_location )
{
}

Node::Node( const SourceLocation& source_location, std::unique_ptr<Node> child )
    : source_location( source_location )
{
  children.reserve( 1 );
  children.push_back( std::move( child ) );
}

Node::Node( const SourceLocation& source_location ) : children(), source_location( source_location )
{
}

std::string Node::describe() const
{
  std::string w;
  describe_to( w );
  return w;
}

std::string Node::to_string_tree() const
{
  return fmt::to_string( *this );
}

void Node::debug( const std::string& msg ) const
{
  source_location.debug( msg );
}

void Node::internal_error( const std::string& msg ) const
{
  source_location.internal_error( msg );
}

void Node::describe_tree_to_indented( const Node& node, std::string& w, unsigned indent )
{
  w += std::string( indent * 2, ' ' ) + "- ";
  node.describe_to( w );
  w += "\n";
  for ( const auto& child : node.children )
  {
    if ( child )
      describe_tree_to_indented( *child, w, indent + 1 );
    else
      w += std::string( ( indent + 1 ) * 2, ' ' ) + "- [deleted]\n";
  }
}

void Node::json_tree( const Node& node, picojson::object& w )
{
  w.insert( std::pair<std::string, std::string>{ "type", node.type() } );
  w["start"] = picojson::value( picojson::object( {
      { "line_number",
        picojson::value( static_cast<double>( node.source_location.range.start.line_number ) ) },
      { "character_column", picojson::value( static_cast<double>(
                                node.source_location.range.start.character_column ) ) },
      { "token_index",
        picojson::value( static_cast<double>( node.source_location.range.start.token_index ) ) },
  } ) );
  w["end"] = picojson::value( picojson::object( {
      { "line_number",
        picojson::value( static_cast<double>( node.source_location.range.end.line_number ) ) },
      { "character_column",
        picojson::value( static_cast<double>( node.source_location.range.end.character_column ) ) },
      { "token_index",
        picojson::value( static_cast<double>( node.source_location.range.end.token_index ) ) },
  } ) );

  node.describe_to( w );

  if ( !node.children.empty() )
  {
    picojson::array json_children;
    for ( const auto& child : node.children )
    {
      picojson::object child_obj;
      if ( child )
      {
        json_tree( *child, child_obj );
        json_children.push_back( picojson::value( child_obj ) );
      }
    }

    w.insert(
        std::pair<std::string, picojson::value>( "children", picojson::value( json_children ) ) );
  }
}

}  // namespace Pol::Bscript::Compiler
