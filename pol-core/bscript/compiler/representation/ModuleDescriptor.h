#ifndef POLSERVER_MODULEDESCRIPTOR_H
#define POLSERVER_MODULEDESCRIPTOR_H

#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class ModuleFunctionDescriptor;

class ModuleDescriptor
{
public:
  ModuleDescriptor( std::string name, std::vector<ModuleFunctionDescriptor> );

  const std::string name;
  const std::vector<ModuleFunctionDescriptor> functions;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_MODULEDESCRIPTOR_H
