#include "VariableAssignmentStatement.h"

#include <format/format.h>

#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
VariableAssignmentStatement::VariableAssignmentStatement( const SourceLocation& source_location,
                                                          std::unique_ptr<Identifier> identifier,
                                                          std::unique_ptr<Node> rhs )
    : Statement( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( identifier ) );
  children.push_back( std::move( rhs ) );
}

void VariableAssignmentStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_variable_assignment_statement( *this );
}

void VariableAssignmentStatement::describe_to( fmt::Writer& w ) const
{
  w << "variable-assignment-statement";
}

Identifier& VariableAssignmentStatement::identifier()
{
  return child<Identifier>( 0 );
}

Node& VariableAssignmentStatement::rhs()
{
  return child<Node>( 1 );
}

}  // namespace Pol::Bscript::Compiler
