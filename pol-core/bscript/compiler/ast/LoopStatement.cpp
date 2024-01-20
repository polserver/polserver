#include "LoopStatement.h"

#include "bscript/compiler/model/FlowControlLabel.h"

namespace Pol::Bscript::Compiler
{
LoopStatement::LoopStatement( const SourceLocation& source_location, std::string label )
    : LabelableStatement( source_location, std::move( label ) ),
      break_label( std::make_shared<FlowControlLabel>() ),
      continue_label( std::make_shared<FlowControlLabel>() )
{
}

}  // namespace Pol::Bscript::Compiler
