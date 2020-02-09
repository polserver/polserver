/** @file
 *
 * @par History
 */


#ifndef FILEACCESS_H
#define FILEACCESS_H

#include "../../clib/maputil.h"

#include <set>
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
class Package;
}

namespace Module
{
class FileAccess
{
public:
  explicit FileAccess( Clib::ConfigElem& elem );
  bool AllowsAccessTo( const Plib::Package* pkg, const Plib::Package* filepackage ) const;
  bool AppliesToPackage( const Plib::Package* pkg ) const;
  bool AppliesToScript( const std::string& scriptname ) const;

  bool AppliesToPath( const std::string& path, const Plib::Package* filepkg ) const;
  size_t estimateSize() const;

  bool AllowWrite;
  bool AllowAppend;
  bool AllowRead;
  bool AllowRemote;
  bool AllowExecute;

  bool AllPackages;
  bool AllDirectories;
  bool AllExtensions;

  std::set<const Plib::Package*> Packages;
  std::vector<std::pair<const Plib::Package*, std::string>> Directories;
  std::vector<std::string> Extensions;
  std::set<std::string> Scripts;
};

bool HasWriteAccess( const Plib::Package* pkg, const Plib::Package* filepackage,
                     const std::string& path );
bool HasExecuteAccess( const Plib::Package* pkg, const std::string& scriptname );
}  // namespace Module
}  // namespace Pol
#endif
