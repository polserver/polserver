#include "ClassDeclarationRegistrar.h"

#include "bscript/compiler/representation/ClassDescriptor.h"

namespace Pol::Bscript::Compiler
{
class MethodDescriptor;

ClassDeclarationRegistrar::ClassDeclarationRegistrar() = default;


void ClassDeclarationRegistrar::register_class(
    unsigned class_name_offset, const std::vector<ConstructorDescriptor>& constructors,
    const std::vector<MethodDescriptor>& method_descriptors )
{
  descriptors.emplace_back( class_name_offset, constructors, method_descriptors );
}

std::vector<ClassDescriptor> ClassDeclarationRegistrar::take_descriptors()
{
  return std::move( descriptors );
}

}  // namespace Pol::Bscript::Compiler
