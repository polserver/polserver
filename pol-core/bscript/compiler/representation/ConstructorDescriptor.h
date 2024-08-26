#pragma once

namespace Pol::Bscript::Compiler
{
class ConstructorDescriptor
{
public:
  ConstructorDescriptor( unsigned function_reference_index );

  const unsigned function_reference_index;
};
}  // namespace Pol::Bscript::Compiler
