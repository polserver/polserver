#include "CaseDispatchGroupVisitor.h"

#include "bscript/compiler/ast/CaseDispatchDefaultSelector.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/codegen/CaseJumpDataBlock.h"
#include "bscript/compiler/model/FlowControlLabel.h"

namespace Pol::Bscript::Compiler
{
CaseDispatchGroupVisitor::CaseDispatchGroupVisitor( CaseJumpDataBlock& data_block,
                                                    FlowControlLabel& group_block_label,
                                                    FlowControlLabel& default_label )
    : data_block( data_block ),
      group_block_label( group_block_label ),
      default_label( default_label )
{
}

void CaseDispatchGroupVisitor::visit_case_dispatch_default_selector( CaseDispatchDefaultSelector& )
{
  default_label = group_block_label;
}

void CaseDispatchGroupVisitor::visit_integer_value( IntegerValue& integer_node )
{
  data_block.on_integer_value_jump_to( integer_node.value, group_block_label.address() );
}

void CaseDispatchGroupVisitor::visit_string_value( StringValue& string_node )
{
  data_block.on_string_value_jump_to( string_node.value, group_block_label.address() );
}

}  // namespace Pol::Bscript::Compiler
