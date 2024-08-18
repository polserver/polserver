#pragma once

#include <vector>

namespace Pol::Bscript::Compiler
{
class ClassDescriptor;

class ClassDeclarationRegistrar
{
public:
  ClassDeclarationRegistrar();


  std::vector<ClassDescriptor> take_descriptors();
};
}  // namespace Pol::Bscript::Compiler
