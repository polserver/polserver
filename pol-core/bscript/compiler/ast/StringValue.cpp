#include "StringValue.h"

#include <format/format.h>

#include "clib/strutil.h"
#include "compiler/ast/NodeVisitor.h"

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
  w << "string-value(" << Clib::getencodedquotedstring( value ) << ")";
}

}  // namespace Pol::Bscript::Compiler