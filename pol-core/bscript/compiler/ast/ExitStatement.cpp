#include "ExitStatement.h"


#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ExitStatement::ExitStatement( const SourceLocation& source_location ) : Statement( source_location )
{
}

void ExitStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_exit_statement( *this );
}

void ExitStatement::describe_to( std::string& w ) const
{
  w += "exit-statement";
}

}  // namespace Pol::Bscript::Compiler
