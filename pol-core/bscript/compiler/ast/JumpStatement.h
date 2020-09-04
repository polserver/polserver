#ifndef POLSERVER_JUMPSTATEMENT_H
#define POLSERVER_JUMPSTATEMENT_H

#include "compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class FlowControlLabel;


class JumpStatement : public Statement
{
public:
  JumpStatement( const SourceLocation&, std::string label );

  const std::string label;

  std::shared_ptr<FlowControlLabel> flow_control_label;
  unsigned local_variables_to_remove;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_JUMPSTATEMENT_H
