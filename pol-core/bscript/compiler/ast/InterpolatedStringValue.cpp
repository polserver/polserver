#include "InterpolatedStringValue.h"

#include <format/format.h>

#include "clib/strutil.h"
#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
InterpolatedStringValue::InterpolatedStringValue( const SourceLocation& source_location,
                                                  std::string value )
    : Value( source_location ), value( std::move( value ) )
{
}

void InterpolatedStringValue::accept( NodeVisitor& visitor )
{
  visitor.visit_interpolated_string_value( *this );
}

void InterpolatedStringValue::describe_to( fmt::Writer& w ) const
{
  w << "string-value(" << Clib::getencodedquotedstring( value ) << ")";
}

}  // namespace Pol::Bscript::Compiler