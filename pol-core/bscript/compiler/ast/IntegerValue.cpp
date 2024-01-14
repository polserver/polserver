#include "IntegerValue.h"


#include "bscript/compiler/ast/NodeVisitor.h"

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

void IntegerValue::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "integer-value({})", value );
}

}  // namespace Pol::Bscript::Compiler
