#include "DoubleKeyword.h"

namespace Pol::Bscript::Compiler
{
DoubleKeyword::DoubleKeyword( const SourceLocation& source_location ) : TypeNode( source_location )
{
}

void DoubleKeyword::describe_to( std::string& w ) const
{
  w += "double-keyword";
}
}  // namespace Pol::Bscript::Compiler
