#include "LabelableStatement.h"

namespace Pol::Bscript::Compiler
{
LabelableStatement::LabelableStatement( const SourceLocation& source_location, std::string label )
    : Statement( source_location ), label( std::move( label ) )
{
}

void LabelableStatement::relabel( std::string new_label )
{
  label = std::move( new_label );
}

}  // namespace Pol::Bscript::Compiler
