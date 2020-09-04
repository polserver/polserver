#ifndef POLSERVER_JUMPSTATEMENT_H
#define POLSERVER_JUMPSTATEMENT_H

#include "compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class FlowControlLabel;


class JumpStatement : public Statement
{
public:
  enum JumpType {
    Break,
    Continue
  };

  JumpStatement( const SourceLocation&, JumpType, std::string label );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  const JumpType jump_type;
  const std::string label;

  std::shared_ptr<FlowControlLabel> flow_control_label;
  unsigned local_variables_to_remove;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_JUMPSTATEMENT_H
