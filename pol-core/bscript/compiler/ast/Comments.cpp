#include "Comments.h"


#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
Comments::Comments( const SourceLocation& source_location )
    : Node( source_location )
{
}

void Comments::accept( NodeVisitor& visitor )
{
  visitor.visit_children( *this );
}

void Comments::describe_to( std::string& w ) const
{
  w += type();
}

std::string Comments::type() const
{
  return "comments";
}

}  // namespace Pol::Bscript::Compiler
