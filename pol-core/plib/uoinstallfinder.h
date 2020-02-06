#ifndef UOINSTALLFINDER_H
#define UOINSTALLFINDER_H

#include <string>
#include <vector>

namespace Pol
{
namespace Clib
{
class ConfigElem;
}
namespace Plib
{
class UOInstallFinder
{
public:
  static std::string getInstallDir();
  static std::string remove_elem( Clib::ConfigElem& elem );
};
}  // namespace Plib
}  // namespace Pol
#endif
