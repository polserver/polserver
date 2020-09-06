#include "ModuleFunctionDescriptor.h"

namespace Pol::Bscript::Compiler
{
ModuleFunctionDescriptor::ModuleFunctionDescriptor( std::string name, size_t parameter_count )
  : name( std::move( name ) ), parameter_count( parameter_count )
{
}

}  // namespace Pol::Bscript::Compiler
