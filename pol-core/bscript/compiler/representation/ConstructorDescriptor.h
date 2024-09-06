#pragma once

namespace Pol::Bscript::Compiler
{
class ConstructorDescriptor
{
public:
  ConstructorDescriptor( unsigned type_tag_offset );

  const unsigned type_tag_offset;
};
}  // namespace Pol::Bscript::Compiler
