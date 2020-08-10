#ifndef POLSERVER_MODULEFUNCTIONDESCRIPTOR_H
#define POLSERVER_MODULEFUNCTIONDESCRIPTOR_H

#include <string>

namespace Pol::Bscript::Compiler
{
class ModuleFunctionDescriptor;

class ModuleFunctionDescriptor
{
public:
  ModuleFunctionDescriptor( std::string, size_t );

  const std::string name;
  const size_t parameter_count;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_MODULEFUNCTIONDESCRIPTOR_H
