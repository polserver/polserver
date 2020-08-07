#ifndef POLSERVER_CASESTATEMENT_H
#define POLSERVER_CASESTATEMENT_H

#include "LabelableStatement.h"

namespace Pol::Bscript::Compiler
{
class CaseDispatchGroup;
class CaseDispatchGroups;
class Expression;
class FlowControlLabel;

class CaseStatement : public LabelableStatement
{
public:
  CaseStatement( const SourceLocation&, std::string label, std::unique_ptr<Expression>,
                 std::unique_ptr<CaseDispatchGroups> );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  Expression& expression();
  CaseDispatchGroups& dispatch_groups_holder();

  const std::shared_ptr<FlowControlLabel> break_label;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CASESTATEMENT_H
