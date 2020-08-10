#include "StringValue.h"

#include <format/format.h>

#include "../clib/strutil.h"

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
StringValue::StringValue( const SourceLocation& source_location, std::string value )
    : Value( source_location ), value( std::move( value ) )
{
}

void StringValue::accept( NodeVisitor& visitor )
{
  visitor.visit_string_value( *this );
}

void StringValue::describe_to( fmt::Writer& w ) const
{
  // why doesn't <iomanip> provide std::quoted?
  w << "string-literal(" << Clib::getencodedquotedstring( value ) << ")";
}

}  // namespace Pol::Bscript::Compiler