#include "ConstructorDescriptor.h"

namespace Pol::Bscript::Compiler
{
ConstructorDescriptor::ConstructorDescriptor( unsigned address, unsigned function_reference_index )
    : address( address ), function_reference_index( function_reference_index )
{
}
}  // namespace Pol::Bscript::Compiler
