#ifndef POLSERVER_CASEDISPATCHGROUPVISITOR_H
#define POLSERVER_CASEDISPATCHGROUPVISITOR_H

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
class CaseJumpDataBlock;
class FlowControlLabel;

class CaseDispatchGroupVisitor : public NodeVisitor
{
public:
  CaseDispatchGroupVisitor( CaseJumpDataBlock& data_block, FlowControlLabel& group_block_label,
                            FlowControlLabel& default_label );

  void visit_case_dispatch_default_selector( CaseDispatchDefaultSelector& ) override;
  void visit_integer_value( IntegerValue& ) override;
  void visit_string_value( StringValue& ) override;

private:
  CaseJumpDataBlock& data_block;
  FlowControlLabel& group_block_label;
  FlowControlLabel& default_label;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CASEDISPATCHGROUPVISITOR_H
