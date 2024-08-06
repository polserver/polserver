#ifndef POLSERVER_NODE_H
#define POLSERVER_NODE_H

#include <fmt/format.h>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "bscript/compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
class Node;
class NodeVisitor;
typedef std::vector<std::unique_ptr<Node>> NodeVector;

template <typename TO, typename FROM>
std::unique_ptr<TO> static_unique_pointer_cast( std::unique_ptr<FROM> old )
{
  return std::unique_ptr<TO>{ static_cast<TO*>( old.release() ) };
}

class Node
{
public:
  explicit Node( const SourceLocation& );
  Node( const SourceLocation&, NodeVector children );
  Node( const SourceLocation&, std::unique_ptr<Node> child );

  template <class T>
  Node( const SourceLocation& source_location, std::vector<std::unique_ptr<T>> ch )
      : source_location( source_location )
  {
    children.reserve( ch.size() );
    for ( auto& child : ch )
    {
      children.push_back( std::move( child ) );
    }
  }

  Node( const Node& ) = delete;
  Node& operator=( const Node& ) = delete;
  virtual ~Node() = default;

  virtual void accept( NodeVisitor& visitor ) = 0;
  virtual void describe_to( std::string& ) const = 0;

  [[nodiscard]] std::string describe() const;
  [[nodiscard]] std::string to_string_tree() const;

  template <class T>
  std::unique_ptr<T> take_child( size_t index )
  {
    if ( index >= children.size() )
      internal_error( "no child at index " + std::to_string( index ) );

    return static_unique_pointer_cast<T, Node>( std::move( children.at( index ) ) );
  }

  template <class T>
  T& child( size_t index )
  {
    if ( index >= children.size() )
      internal_error( "no child at index " + std::to_string( index ) );
    return static_cast<T&>( *children.at( index ) );
  }

  template <typename T>
  void get_children( std::vector<std::reference_wrapper<T>>& result ) const
  {
    for ( auto& child : children )
    {
      if ( auto casted = dynamic_cast<T*>( child.get() ) )
      {
        result.emplace_back( *casted );
      }
    }
  }

  NodeVector children;

  const SourceLocation source_location;

  void debug( const std::string& msg ) const;
  [[noreturn]] void internal_error( const std::string& msg ) const;

  static void describe_tree_to_indented( const Node&, std::string& w, unsigned indent );
};


}  // namespace Pol::Bscript::Compiler
template <typename T>
struct fmt::formatter<
    T, std::enable_if_t<std::is_base_of<Pol::Bscript::Compiler::Node, T>::value, char>>
    : fmt::formatter<std::string>
{
  inline fmt::format_context::iterator format( const Pol::Bscript::Compiler::Node& n,
                                               fmt::format_context& ctx ) const
  {
    std::string w;
    n.describe_tree_to_indented( n, w, 0 );
    return fmt::formatter<std::string>::format( w, ctx );
  }
};

#endif  // POLSERVER_NODE_H
