#include "ClassBody.h"

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ClassBody::ClassBody( const SourceLocation& source_location, NodeVector statements )
    : Statement( source_location )
{
  children.reserve( statements.size() );
  for ( auto& statement : statements )
  {
    children.push_back( std::move( statement ) );
  }
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
