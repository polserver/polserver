
#ifndef MODMETHODS_H
#define MODMETHODS_H

#include <map>

namespace Pol
{
namespace Bscript
{
typedef struct
{
  int funcIdx;
  int argc;
} ModuleMethod;


typedef std::map<std::string, std::map<std::string, ModuleMethod>> CoreModuleContainer;
extern CoreModuleContainer module_methods;


ModuleMethod* getKnownModuleMethod( const char* mod, const char* method );
}  // namespace Bscript
}  // namespace Pol
#endif
