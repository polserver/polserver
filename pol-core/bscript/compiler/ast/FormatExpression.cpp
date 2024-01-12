#include "FormatExpression.h"

#include <format/format.h>

#include "clib/strutil.h"
#include "compiler/ast/NodeVisitor.h"
#include "compiler/ast/StringValue.h"

namespace Pol::Bscript::Compiler
{
FormatExpression::FormatExpression( const SourceLocation& source_location,
                                    std::unique_ptr<Expression> expression,
                                    std::unique_ptr<StringValue> format )
    : Expression( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( expression ) );
  children.push_back( std::move( format ) );
}

void FormatExpression::accept( NodeVisitor& visitor )
{
  visitor.visit_format_expression( *this );
}

void FormatExpression::describe_to( std::string& w ) const
{
  w += "format-expression";
}

}  // namespace Pol::Bscript::Compiler
