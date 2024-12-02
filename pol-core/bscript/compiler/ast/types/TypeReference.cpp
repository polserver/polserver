#include "TypeReference.h"

#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
TypeReference::TypeReference( const SourceLocation& source_location, std::string type_name )
    : TypeNode( source_location ), type_name( std::move( type_name ) )
{
}

void TypeReference::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "type-reference({})",
                  Clib::getencodedquotedstring( type_name ) );
}
}  // namespace Pol::Bscript::Compiler
