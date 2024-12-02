#include "ArrayType.h"

namespace Pol::Bscript::Compiler
{
ArrayType::ArrayType( const SourceLocation& source_location ) : TypeNode( source_location ) {}

ArrayType::ArrayType( const SourceLocation&, std::unique_ptr<TypeNode> element_type )
    : TypeNode( source_location, std::move( element_type ) )
{
}

void ArrayType::describe_to( std::string& w ) const
{
  w += "array-type";
}
}  // namespace Pol::Bscript::Compiler
