#include "RegularExpressionValue.h"


#include "bscript/compiler/ast/NodeVisitor.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
RegularExpressionValue::RegularExpressionValue( const SourceLocation& source_location,
                                                std::string pattern, std::string flags )
    : Value( source_location ), pattern( std::move( pattern ) ), flags( std::move( flags ) )
{
}

void RegularExpressionValue::accept( NodeVisitor& visitor )
{
  visitor.visit_regular_expression_value( *this );
}

void RegularExpressionValue::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "regular-expression-value({}, {})", pattern, flags );
}

}  // namespace Pol::Bscript::Compiler