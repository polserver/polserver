#include "UnionType.h"

namespace Pol::Bscript::Compiler
{
UnionType::UnionType( const SourceLocation& source_location, std::unique_ptr<TypeNode> lhs,
                      std::unique_ptr<TypeNode> rhs )
    : TypeNode( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( lhs ) );
  children.push_back( std::move( rhs ) );
}

void UnionType::describe_to( std::string& w ) const
{
  w += "union-type";
}
}  // namespace Pol::Bscript::Compiler
