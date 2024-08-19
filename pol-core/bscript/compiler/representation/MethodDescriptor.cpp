#include "MethodDescriptor.h"

namespace Pol::Bscript::Compiler
{
MethodDescriptor::MethodDescriptor( unsigned name_offset, unsigned address,
                                    unsigned function_reference_index )
    : name_offset( name_offset ),
      address( address ),
      function_reference_index( function_reference_index )
{
}
}  // namespace Pol::Bscript::Compiler
