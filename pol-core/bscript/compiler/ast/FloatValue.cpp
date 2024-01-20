#include "FloatValue.h"


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

void FloatValue::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "float-value({})", value );
}

}  // namespace Pol::Bscript::Compiler
