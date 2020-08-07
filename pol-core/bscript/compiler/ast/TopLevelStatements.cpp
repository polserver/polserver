#include "TopLevelStatements.h"

#include <format/format.h>

#include "NodeVisitor.h"
#include "Statement.h"

namespace Pol::Bscript::Compiler
{
TopLevelStatements::TopLevelStatements( const SourceLocation& source_location,
                                        std::vector<std::unique_ptr<Statement>> statements )
    : Node( source_location, std::move( statements ) )
{
}

void TopLevelStatements::accept( NodeVisitor& visitor )
{
  visitor.visit_top_level_statements( *this );
}

void TopLevelStatements::describe_to( fmt::Writer& w ) const
{
  w << "top-level-statements";
}

}  // namespace Pol::Bscript::Compiler
