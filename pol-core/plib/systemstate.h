
#ifndef PLIB_SYTEMSTATE_H
#define PLIB_SYTEMSTATE_H

#include <map>
#include <vector>
#if !defined( _WIN32 )
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "../clib/maputil.h"
#include "../clib/rawtypes.h"
#include "polcfg.h"


namespace Pol::Plib
{
class Tile;
class Package;

using Packages = std::vector<Package*>;
using PackagesByName = std::map<std::string, Package*, Clib::ci_cmp_pred>;

class SystemState
{
public:
  SystemState();
  ~SystemState() = default;
  SystemState( const SystemState& ) = delete;
  SystemState& operator=( const SystemState& ) = delete;
  void deinitialize();

  // TODO: why two containers? keep the map when often name is searched?
  Packages packages;
  PackagesByName packages_byname;

  bool accounts_txt_dirty;
  struct stat accounts_txt_stat;

  PolConfig config;
  std::vector<Tile> tile;
  u16 max_graphic;

  size_t estimatedSize() const;

private:
};

extern SystemState systemstate;
}  // namespace Pol::Plib

#endif
