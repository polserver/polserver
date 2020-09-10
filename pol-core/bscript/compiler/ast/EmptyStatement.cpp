#include "EmptyStatement.h"

#include <format/format.h>

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
EmptyStatement::EmptyStatement( const SourceLocation& source_location )
    : Statement( source_location )
{
}

void EmptyStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_empty_statement( *this );
}

void EmptyStatement::describe_to( fmt::Writer& w ) const
{
  w << "empty-statement";
}

}  // namespace Pol::Bscript::Compiler
