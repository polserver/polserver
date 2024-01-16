#include "TopLevelStatements.h"


#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
TopLevelStatements::TopLevelStatements( const SourceLocation& source_location )
    : Node( source_location )
{
}

void TopLevelStatements::accept( NodeVisitor& visitor )
{
  visitor.visit_top_level_statements( *this );
}

void TopLevelStatements::describe_to( std::string& w ) const
{
  w += "top-level-statements";
}

}  // namespace Pol::Bscript::Compiler
