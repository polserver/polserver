#include "ClassDescriptor.h"

namespace Pol::Bscript::Compiler
{
ClassDescriptor::ClassDescriptor( unsigned name_offset, std::vector<ConstructorDescriptor> constructors,
                                  std::vector<MethodDescriptor> methods )
    : name_offset( name_offset ),
      constructors( std::move( constructors ) ),
      methods( std::move( methods ) )
{
}
}  // namespace Pol::Bscript::Compiler
