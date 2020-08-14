#include "ModuleDescriptor.h"

#include "compiler/representation/ModuleFunctionDescriptor.h"

namespace Pol::Bscript::Compiler
{
ModuleDescriptor::ModuleDescriptor( std::string name,
                                    std::vector<ModuleFunctionDescriptor> functions )
  : name( std::move( name ) ), functions( std::move( functions ) )
{
}

}  // namespace Pol::Bscript::Compiler
