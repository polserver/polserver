#ifndef POLSERVER_LOOPSTATEMENT_H
#define POLSERVER_LOOPSTATEMENT_H

#include "LabelableStatement.h"

namespace Pol::Bscript::Compiler
{
class FlowControlLabel;

class LoopStatement : public LabelableStatement
{
public:
  LoopStatement( const SourceLocation& source_location, std::string label );

  const std::shared_ptr<FlowControlLabel> break_label;
  const std::shared_ptr<FlowControlLabel> continue_label;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_LOOPSTATEMENT_H
