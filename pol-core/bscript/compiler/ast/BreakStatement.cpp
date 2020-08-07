#include "BreakStatement.h"

#include <format/format.h>
#include <utility>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
BreakStatement::BreakStatement( const SourceLocation& source_location, std::string label )
    : JumpStatement( source_location, std::move( label ) )
{
}

void BreakStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_break_statement( *this );
}

void BreakStatement::describe_to( fmt::Writer& w ) const
{
  w << "break-statement(" << label << ")";
}

}  // namespace Pol::Bscript::Compiler
