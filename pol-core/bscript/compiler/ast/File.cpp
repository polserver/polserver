#include "File.h"


#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
File::File( const SourceLocation& source_location )
    : Node( source_location )
{
}

void File::accept( NodeVisitor& visitor )
{
  visitor.visit_children( *this );
}

void File::describe_to( std::string& w ) const
{
  w += type();
}

std::string File::type() const
{
  return "file";
}

}  // namespace Pol::Bscript::Compiler
