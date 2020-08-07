#include "ExitStatement.h"

#include <format/format.h>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ExitStatement::ExitStatement( const SourceLocation& source_location ) : Statement( source_location )
{
}

void ExitStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_exit_statement( *this );
}

void ExitStatement::describe_to( fmt::Writer& w ) const
{
  w << "exit-statement";
}

}  // namespace Pol::Bscript::Compiler
