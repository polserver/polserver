#include "IntegerKeyword.h"

namespace Pol::Bscript::Compiler
{
IntegerKeyword::IntegerKeyword( const SourceLocation& source_location )
    : TypeNode( source_location )
{
}

void IntegerKeyword::describe_to( std::string& w ) const
{
  w += "integer-keyword";
}
}  // namespace Pol::Bscript::Compiler
