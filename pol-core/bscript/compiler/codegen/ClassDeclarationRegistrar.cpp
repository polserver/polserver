#include "ClassDeclarationRegistrar.h"

#include "bscript/compiler/representation/ClassDescriptor.h"
#include "bscript/compiler/representation/MethodDescriptor.h"

namespace Pol::Bscript::Compiler
{
class MethodDescriptor;

ClassDeclarationRegistrar::ClassDeclarationRegistrar() = default;


void ClassDeclarationRegistrar::register_class(
    unsigned class_name_offset, const std::vector<unsigned>& constructor_addresses,
    const std::vector<MethodDescriptor>& method_descriptors )
{
  descriptors.emplace_back( class_name_offset, constructor_addresses, method_descriptors );
}

std::vector<ClassDescriptor> ClassDeclarationRegistrar::take_descriptors()
{
  return std::move( descriptors );
}

}  // namespace Pol::Bscript::Compiler
