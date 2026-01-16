/** @file
 *
 * @par History
 */


#ifndef MISCRGN_H
#define MISCRGN_H


#include "base/range.h"
#include "regions/region.h"
#include "zone.h"


namespace Pol::Clib
{
class ConfigElem;
}  // namespace Pol::Clib


namespace Pol
{
namespace Realms
{
class Realm;
}
namespace Core
{
class NoCastRegion final : public Region
{
  using base = Region;

public:
  NoCastRegion( Clib::ConfigElem& elem, RegionId id );
  size_t estimateSize() const override;
  bool nocast() const;

private:
  bool nocast_;
};

inline bool NoCastRegion::nocast() const
{
  return nocast_;
}


class LightRegion final : public Region
{
  using base = Region;

public:
  LightRegion( Clib::ConfigElem& elem, RegionId id );
  size_t estimateSize() const override;
  unsigned lightlevel;
};


class WeatherRegion final : public Region
{
  using base = Region;

public:
  WeatherRegion( Clib::ConfigElem& elem, RegionId id );
  size_t estimateSize() const override;
  void setweather( unsigned char weathertype, unsigned char severity, unsigned char aux );
  unsigned char weathertype;
  unsigned char severity;
  unsigned char aux;
  int lightoverride;  // if -1, no override
};

class WeatherDef final : public RegionGroup<WeatherRegion>
{
public:
  WeatherDef( const char* name );
  ~WeatherDef() override;
  void copy_default_regions();
  size_t estimateSize() const override;

  bool assign_zones_to_region( const char* regionname, const Range2d& area, Realms::Realm* realm );

private:
  RegionRealms default_regionrealms;  // y first
};
}  // namespace Core
}  // namespace Pol

#endif
