#ifndef POLSERVER_MODULEDESCRIPTOR_H
#define POLSERVER_MODULEDESCRIPTOR_H

#include <string>
#include <vector>

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
class ModuleFunctionDescriptor;

class ModuleDescriptor
{
public:
  ModuleDescriptor( std::string name, std::vector<ModuleFunctionDescriptor> );

  const std::string name;
  const std::vector<ModuleFunctionDescriptor> functions;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_MODULEDESCRIPTOR_H
