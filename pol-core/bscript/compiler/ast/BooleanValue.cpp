#include "BooleanValue.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
BooleanValue::BooleanValue( const SourceLocation& source_location, bool value )
    : Value( source_location ), value( value )
{
}

void BooleanValue::accept( NodeVisitor& visitor )
{
  visitor.visit_boolean_value( *this );
}

void BooleanValue::describe_to( fmt::Writer& w ) const
{
  w << "boolean-value(" << value << ")";
}

}  // namespace Pol::Bscript::Compiler
