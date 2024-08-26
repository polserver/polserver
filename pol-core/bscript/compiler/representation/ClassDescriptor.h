#pragma once

#include <string>
#include <vector>

#include "bscript/compiler/representation/ConstructorDescriptor.h"
#include "bscript/compiler/representation/MethodDescriptor.h"

namespace Pol::Bscript::Compiler
{
class ClassDescriptor
{
public:
  ClassDescriptor( unsigned name_offset, std::vector<ConstructorDescriptor> constructors,
                   std::vector<MethodDescriptor> methods );

  const unsigned name_offset;
  const std::vector<ConstructorDescriptor> constructors;
  const std::vector<MethodDescriptor> methods;
};

}  // namespace Pol::Bscript::Compiler
