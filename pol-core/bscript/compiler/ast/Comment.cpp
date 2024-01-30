#include "Comment.h"


#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
Comment::Comment( const SourceLocation& source_location )
    : Node( source_location )
{
}

void Comment::accept( NodeVisitor& visitor )
{
  visitor.visit_children( *this );
}

void Comment::describe_to( std::string& w ) const
{
  w += type();
}

std::string Comment::type() const
{
  return "comment";
}

}  // namespace Pol::Bscript::Compiler
