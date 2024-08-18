#include "ClassDeclarationRegistrar.h"

#include "bscript/compiler/representation/ClassDescriptor.h"

namespace Pol::Bscript::Compiler
{
ClassDeclarationRegistrar::ClassDeclarationRegistrar() = default;

std::vector<ClassDescriptor> ClassDeclarationRegistrar::take_descriptors()
{
  std::vector<ClassDescriptor> class_descriptors;
  return class_descriptors;
}

}  // namespace Pol::Bscript::Compiler
