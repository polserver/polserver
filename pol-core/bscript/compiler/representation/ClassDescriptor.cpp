#include "ClassDescriptor.h"

#include "bscript/compiler/representation/MethodDescriptor.h"

namespace Pol::Bscript::Compiler
{
ClassDescriptor::ClassDescriptor( unsigned name_offset, std::vector<unsigned> constructor_addresses,
                                  std::vector<MethodDescriptor> methods )
    : name_offset( name_offset ),
      constructor_addresses( std::move( constructor_addresses ) ),
      methods( std::move( methods ) )
{
}
}  // namespace Pol::Bscript::Compiler
