#include "TupleType.h"

namespace Pol::Bscript::Compiler
{
TupleType::TupleType( const SourceLocation&, NodeVector elements )
    : TypeNode( source_location, std::move( elements ) )
{
}

void TupleType::describe_to( std::string& w ) const
{
  w += "tuple-type";
}
}  // namespace Pol::Bscript::Compiler
