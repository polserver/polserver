#include "ModuleDescriptor.h"

#include "ModuleFunctionDescriptor.h"

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
ModuleDescriptor::ModuleDescriptor( std::string name,
                                    std::vector<ModuleFunctionDescriptor> functions )
  : name( std::move( name ) ), functions( std::move( functions ) )
{
}

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol
