/** @file
 *
 * @par History
 */


#ifndef FILEACCESS_H
#define FILEACCESS_H

#include "../../clib/maputil.h"

#include <vector>
#include <set>
#include <string>

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
  bool AppliesToPath( const std::string& path ) const;
  size_t estimateSize() const;

  bool AllowWrite;
  bool AllowAppend;
  bool AllowRead;

  bool AllowRemote;

  bool AllPackages;
  bool AllDirectories; // not used
  bool AllExtensions;

  std::set<std::string, Clib::ci_cmp_pred> Packages;
  std::vector< std::string > Directories; // not used
  std::vector< std::string > Extensions;
};

bool HasWriteAccess( const Plib::Package* pkg, const Plib::Package* filepackage, const std::string& path );
}
}
#endif
