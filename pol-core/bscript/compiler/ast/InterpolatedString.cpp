#include "InterpolatedString.h"

#include <format/format.h>

#include "clib/strutil.h"
#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
InterpolatedString::InterpolatedString( const SourceLocation& source_location,
                                        std::vector<std::unique_ptr<Expression>> children )
    : Expression( source_location, std::move( children ) )
{
}

void InterpolatedString::accept( NodeVisitor& visitor )
{
  visitor.visit_interpolated_string( *this );
}

void InterpolatedString::describe_to( fmt::Writer& w ) const
{
  w << "interpolated-string (" << children.size() << " parts)";
}

}  // namespace Pol::Bscript::Compiler