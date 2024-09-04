#include "FlowControlLabel.h"

#include <stdexcept>

#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/UserFunction.h"

namespace Pol::Bscript::Compiler
{
FlowControlLabel::FlowControlLabel() : maybe_address(), referencing_instruction_addresses() {}

bool FlowControlLabel::has_address() const
{
  return maybe_address.has_value();
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
  if ( maybe_address.has_value() )
    throw std::runtime_error( "Label address assigned twice" );
  maybe_address = address;
}

void FlowControlLabel::add_referencing_instruction_address( unsigned address )
{
  referencing_instruction_addresses.push_back( address );
}

std::string FlowControlLabel::label_for_user_function_default_argument(
    const UserFunction& uf, const FunctionParameterDeclaration& param )
{
  return fmt::format( "{}#{}", uf.scoped_name(), param.name.string() );
}

}  // namespace Pol::Bscript::Compiler
