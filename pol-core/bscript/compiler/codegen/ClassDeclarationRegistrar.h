#pragma once

#include <vector>

namespace Pol::Bscript::Compiler
{
class ClassDescriptor;
class ConstructorDescriptor;
class MethodDescriptor;

class ClassDeclarationRegistrar
{
public:
  ClassDeclarationRegistrar();

  void register_class( unsigned class_name_offset, unsigned constructor_function_reference_index,
                       const std::vector<ConstructorDescriptor>& constructor_descriptors,
                       const std::vector<MethodDescriptor>& method_descriptors );

  std::vector<ClassDescriptor> take_descriptors();

private:
  std::vector<ClassDescriptor> descriptors;
};
}  // namespace Pol::Bscript::Compiler
