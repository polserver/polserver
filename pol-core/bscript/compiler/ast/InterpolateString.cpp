#include "InterpolateString.h"

#include <format/format.h>

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

void InterpolateString::describe_to( fmt::Writer& w ) const
{
  w << "interpolate-string (" << children.size() << " parts)";
}

}  // namespace Pol::Bscript::Compiler
