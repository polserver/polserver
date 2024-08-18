#pragma once

#include <vector>

namespace Pol::Bscript::Compiler
{
class ClassDescriptor;
class MethodDescriptor;

class ClassDeclarationRegistrar
{
public:
  ClassDeclarationRegistrar();

  void register_class( unsigned class_name_offset,
                       const std::vector<unsigned>& constructor_addresses,
                       const std::vector<MethodDescriptor>& method_descriptors );

  std::vector<ClassDescriptor> take_descriptors();

private:
  std::vector<ClassDescriptor> descriptors;
};
}  // namespace Pol::Bscript::Compiler
