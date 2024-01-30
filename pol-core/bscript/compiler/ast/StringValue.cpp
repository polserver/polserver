#include "StringValue.h"


#include "bscript/compiler/ast/NodeVisitor.h"
#include "clib/strutil.h"

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

void StringValue::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "{}({})", type(),
                  Clib::getencodedquotedstring( value ) );
}

void StringValue::describe_to( picojson::object& o ) const {
  o["value"] = picojson::value( value );
}

std::string StringValue::type() const
{
  return "string-value";
}

}  // namespace Pol::Bscript::Compiler