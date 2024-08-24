#pragma once

namespace Pol::Bscript::Compiler
{
class ConstructorDescriptor
{
public:
  ConstructorDescriptor( unsigned address, unsigned function_reference_index );

  const unsigned address;
  const unsigned function_reference_index;
};
}  // namespace Pol::Bscript::Compiler
