#pragma once

#include <string>

namespace Pol::Bscript::Compiler
{
class ClassDescriptor
{
public:
  ClassDescriptor( std::string );

  const std::string name;
};

}  // namespace Pol::Bscript::Compiler
