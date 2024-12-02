#include "IntersectionType.h"

namespace Pol::Bscript::Compiler
{
IntersectionType::IntersectionType( const SourceLocation& source_location,
                                    std::unique_ptr<TypeNode> lhs, std::unique_ptr<TypeNode> rhs )
    : TypeNode( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( lhs ) );
  children.push_back( std::move( rhs ) );
}

void IntersectionType::describe_to( std::string& w ) const
{
  w += "intersection-type";
}
}  // namespace Pol::Bscript::Compiler
