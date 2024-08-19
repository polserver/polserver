#pragma once

#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class MethodDescriptor;
class ClassDescriptor
{
public:
  ClassDescriptor( unsigned name_offset, std::vector<unsigned> constructor_addresses,
                   std::vector<MethodDescriptor> methods );

  const unsigned name_offset;
  const std::vector<unsigned> constructor_addresses;
  const std::vector<MethodDescriptor> methods;
};

}  // namespace Pol::Bscript::Compiler
