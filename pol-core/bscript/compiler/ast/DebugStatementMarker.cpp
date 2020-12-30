#include "DebugStatementMarker.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
DebugStatementMarker::DebugStatementMarker( const SourceLocation& location, std::string text,
                                            unsigned start_index )
    : Statement( location ), text( std::move( text ) ), start_index( start_index )
{
}

void DebugStatementMarker::accept( NodeVisitor& visitor )
{
  visitor.visit_debug_statement_marker( *this );
}

void DebugStatementMarker::describe_to( fmt::Writer& w ) const
{
  w << "debug-statement-marker";
}

}  // namespace Pol::Bscript::Compiler
