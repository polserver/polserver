#include "ModuleFunctionDescriptor.h"

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
ModuleFunctionDescriptor::ModuleFunctionDescriptor( std::string name, size_t parameter_count )
  : name( std::move( name ) ), parameter_count( parameter_count )
{
}

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol
