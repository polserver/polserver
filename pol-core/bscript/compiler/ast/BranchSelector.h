#ifndef POLSERVER_BRANCHSELECTOR_H
#define POLSERVER_BRANCHSELECTOR_H

#include "compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Expression;
class FlowControlLabel;

class BranchSelector : public Node
{
public:
  enum BranchType {
    IfTrue,
    IfFalse,
    Always,
    Never
  };

  BranchSelector( const SourceLocation&, BranchType branch_type, std::unique_ptr<Expression> );
  BranchSelector( const SourceLocation&, BranchType branch_type );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  const BranchType branch_type;
  std::shared_ptr<FlowControlLabel> flow_control_label;

  Expression* predicate();
};

}

#endif  // POLSERVER_BRANCHSELECTOR_H
