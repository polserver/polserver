#include "AssignVariableConsume.h"

#include <format/format.h>

#include "compiler/ast/Identifier.h"
#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
AssignVariableConsume::AssignVariableConsume( const SourceLocation& source_location,
                                              std::unique_ptr<Identifier> identifier,
                                              std::unique_ptr<Node> rhs )
    : Statement( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( identifier ) );
  children.push_back( std::move( rhs ) );
}

void AssignVariableConsume::accept( NodeVisitor& visitor )
{
  visitor.visit_assign_variable_consume( *this );
}

void AssignVariableConsume::describe_to( fmt::Writer& w ) const
{
  w << "assign-variable-consume";
}

Identifier& AssignVariableConsume::identifier()
{
  return child<Identifier>( 0 );
}

Node& AssignVariableConsume::rhs()
{
  return child<Node>( 1 );
}

}  // namespace Pol::Bscript::Compiler
