#include "BooleanValue.h"

#include <fmt/format.h>

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

void BooleanValue::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "boolean-value({})", value );
}

}  // namespace Pol::Bscript::Compiler
