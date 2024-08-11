#include "ClassBody.h"

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ClassBody::ClassBody( const SourceLocation& source_location )
    : Statement( source_location )
{
}

void ClassBody::accept( NodeVisitor& visitor )
{
  visitor.visit_class_body( *this );
}

void ClassBody::describe_to( std::string& w ) const
{
  w += "class-block";
}

}  // namespace Pol::Bscript::Compiler
