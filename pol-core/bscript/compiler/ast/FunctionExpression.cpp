#include "FunctionExpression.h"

#include <fmt/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
FunctionExpression::FunctionExpression( const SourceLocation& source_location, bool value )
    : Value( source_location ), value( value )
{
}

void FunctionExpression::accept( NodeVisitor& visitor )
{
  visitor.visit_function_expression( *this );
}

void FunctionExpression::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "boolean-value({})", value );
}

}  // namespace Pol::Bscript::Compiler
