#include "InterpolateString.h"


#include "clib/strutil.h"
#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
InterpolateString::InterpolateString( const SourceLocation& source_location,
                                      std::vector<std::unique_ptr<Expression>> children )
    : Expression( source_location, std::move( children ) )
{
}

void InterpolateString::accept( NodeVisitor& visitor )
{
  visitor.visit_interpolate_string( *this );
}

void InterpolateString::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "interpolate-string ({} parts)", children.size() );
}

}  // namespace Pol::Bscript::Compiler
