#include "JumpStatement.h"


#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
JumpStatement::JumpStatement( const SourceLocation& source_location, JumpType jump_type,
                              std::string label )
    : Statement( source_location ),
      jump_type( jump_type ),
      label( std::move( label ) ),
      flow_control_label(),
      local_variables_to_remove( 0 )
{
}

void JumpStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_jump_statement( *this );
}

void JumpStatement::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "{}",
                  ( jump_type == Break ? "break-statement" : "continue-statement" ) );
  if ( !label.empty() )
    fmt::format_to( std::back_inserter( w ), "({})", label );
}

}  // namespace Pol::Bscript::Compiler
