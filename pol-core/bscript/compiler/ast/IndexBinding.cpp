#include "IndexBinding.h"

#include <boost/range/adaptor/sliced.hpp>

#include "bscript/compiler/ast/ElementIndexes.h"
#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "clib/clib.h"

namespace Pol::Bscript::Compiler
{
IndexBinding::IndexBinding( const SourceLocation& loc, std::unique_ptr<ElementIndexes> indices,
                            std::vector<std::unique_ptr<Node>> bindings )
    : Node( loc )
{
  children.reserve( bindings.size() + 1 );
  children.push_back( std::move( indices ) );
  children.insert( children.end(), std::make_move_iterator( bindings.begin() ),
                   std::make_move_iterator( bindings.end() ) );
}
void IndexBinding::accept( NodeVisitor& visitor )
{
  visitor.visit_index_binding( *this );
}

void IndexBinding::describe_to( std::string& w ) const
{
  w += "index-binding";
}

ElementIndexes& IndexBinding::indexes()
{
  return child<ElementIndexes>( 0 );
}

std::vector<std::reference_wrapper<Node>> IndexBinding::bindings()
{
  std::vector<std::reference_wrapper<Node>> params;
  for ( auto& param : children | boost::adaptors::sliced( 1, children.size() ) )
  {
    params.emplace_back( *param.get() );
  }
  return params;
}

u8 IndexBinding::binding_count() const
{
  // Subtract one, for the ElementIndexes at the first child.
  return Clib::clamp_convert<u8>( children.size() - 1 );
}

}  // namespace Pol::Bscript::Compiler
