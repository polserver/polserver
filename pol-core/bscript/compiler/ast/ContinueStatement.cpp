#include "ContinueStatement.h"

#include <format/format.h>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ContinueStatement::ContinueStatement( const SourceLocation& source_location, std::string label )
    : JumpStatement( source_location, std::move( label ) )
{
}

void ContinueStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_continue_statement( *this );
}

void ContinueStatement::describe_to( fmt::Writer& w ) const
{
  w << "continue-statement;";
  if ( !label.empty() )
    w << "(" << label << ")";
}

}  // namespace Pol::Bscript::Compiler
