#include "StructType.h"

namespace Pol::Bscript::Compiler
{
StructType::StructType( const SourceLocation& source_location ) : TypeNode( source_location ) {}

StructType::StructType( const SourceLocation&, NodeVector members )
    : TypeNode( source_location, std::move( members ) )
{
}

void StructType::describe_to( std::string& w ) const
{
  w += "struct-type";
}
}  // namespace Pol::Bscript::Compiler
