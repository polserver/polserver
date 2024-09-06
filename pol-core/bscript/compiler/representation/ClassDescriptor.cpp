#include "ClassDescriptor.h"

namespace Pol::Bscript::Compiler
{
ClassDescriptor::ClassDescriptor( unsigned name_offset,
                                  unsigned constructor_function_reference_index,
                                  std::vector<ConstructorDescriptor> constructors,
                                  std::vector<MethodDescriptor> methods )
    : name_offset( name_offset ),
      constructor_function_reference_index( constructor_function_reference_index ),
      constructors( std::move( constructors ) ),
      methods( std::move( methods ) )
{
}
}  // namespace Pol::Bscript::Compiler
