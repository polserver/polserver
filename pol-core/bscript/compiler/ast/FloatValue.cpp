#include "FloatValue.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
FloatValue::FloatValue( const SourceLocation& source_location, double value )
    : Value( source_location ), value( value )
{
}

void FloatValue::accept( NodeVisitor& visitor )
{
  visitor.visit_float_value( *this );
}

void FloatValue::describe_to( fmt::Writer& w ) const
{
  w << "float-value(" << value << ")";
}

}  // namespace Pol::Bscript::Compiler
