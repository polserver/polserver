#include "FlowControlLabel.h"

namespace Pol::Bscript::Compiler
{
FlowControlLabel::FlowControlLabel() : maybe_address(), referencing_instruction_addresses() {}

bool FlowControlLabel::has_address() const
{
  return maybe_address.is_initialized();
}

unsigned FlowControlLabel::address() const
{
  return maybe_address.value();
}

const std::vector<unsigned>& FlowControlLabel::get_referencing_instruction_addresses() const
{
  return referencing_instruction_addresses;
}

void FlowControlLabel::assign_address( unsigned address )
{
  if ( maybe_address.is_initialized() )
    throw std::runtime_error( "Label address assigned twice" );
  maybe_address = address;
}

void FlowControlLabel::add_referencing_instruction_address( unsigned address )
{
  referencing_instruction_addresses.push_back( address );
}

}  // namespace Pol::Bscript::Compiler
