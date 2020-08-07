#include "IntegerValue.h"

#include <format/format.h>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
IntegerValue::IntegerValue( const SourceLocation& source_location, int value )
    : Value( source_location ), value( value )
{
}

void IntegerValue::accept( NodeVisitor& visitor )
{
  visitor.visit_integer_value( *this );
}

void IntegerValue::describe_to( fmt::Writer& w ) const
{
  w << "integer-literal(" << value << ")";
}

}  // namespace Pol::Bscript::Compiler
