#include "JumpStatement.h"

namespace Pol::Bscript::Compiler
{
JumpStatement::JumpStatement( const SourceLocation& source_location, std::string label )
  : Statement( source_location ),
    label( std::move( label ) ),
    flow_control_label(),
    local_variables_to_remove( 0 )
{
}

}  // namespace Pol::Bscript::Compiler
