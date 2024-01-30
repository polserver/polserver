#include "LineComment.h"


#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
LineComment::LineComment( const SourceLocation& source_location )
    : Node( source_location )
{
}

void LineComment::accept( NodeVisitor& visitor )
{
  visitor.visit_children( *this );
}

void LineComment::describe_to( std::string& w ) const
{
  w += type();
}

std::string LineComment::type() const
{
  return "line-comment";
}

}  // namespace Pol::Bscript::Compiler
