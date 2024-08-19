#pragma once

namespace Pol::Bscript::Compiler
{
class MethodDescriptor
{
public:
  MethodDescriptor( unsigned name_offset, unsigned address, unsigned function_reference_index);

  const unsigned name_offset;
  const unsigned address;
  const unsigned function_reference_index;
};
}  // namespace Pol::Bscript::Compiler
