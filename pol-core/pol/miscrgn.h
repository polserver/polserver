/** @file
 *
 * @par History
 */


#ifndef MISCRGN_H
#define MISCRGN_H


#include "base/position.h"
#include "region.h"
#include "zone.h"

namespace Pol
{
namespace Clib
{
class ConfigElem;
}  // namespace Clib
}  // namespace Pol

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
  typedef Region base;

public:
  NoCastRegion( Clib::ConfigElem& elem, RegionId id );
  virtual size_t estimateSize() const override;
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
  typedef Region base;

public:
  LightRegion( Clib::ConfigElem& elem, RegionId id );
  virtual size_t estimateSize() const override;
  unsigned lightlevel;
};


class WeatherRegion final : public Region
{
  typedef Region base;

public:
  WeatherRegion( Clib::ConfigElem& elem, RegionId id );
  virtual size_t estimateSize() const override;
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
  virtual ~WeatherDef();
  void copy_default_regions();
  virtual size_t estimateSize() const override;

  bool assign_zones_to_region( const char* regionname, Pos2d p1, Pos2d p2, Realms::Realm* realm );

private:
  RegionRealms default_regionrealms;
};
}  // namespace Core
}  // namespace Pol

#endif
