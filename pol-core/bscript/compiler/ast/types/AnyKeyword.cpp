#include "AnyKeyword.h"

namespace Pol::Bscript::Compiler
{
AnyKeyword::AnyKeyword( const SourceLocation& source_location ) : TypeNode( source_location ) {}

void AnyKeyword::describe_to( std::string& w ) const
{
  w += "any-keyword";
}
}  // namespace Pol::Bscript::Compiler
