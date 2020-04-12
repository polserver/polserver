/** @file
 *
 * @par History
 * - 2005/01/23 Shinigami: WeatherDef::Con-/Destructor & WeatherDef::copy_default_regions - Tokuno
 * MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)
 */


#include "miscrgn.h"

#include <stddef.h>
#include <tuple>

#include "../clib/cfgelem.h"
#include "../clib/rawtypes.h"
#include "globals/uvars.h"
#include "realms/realm.h"
#include "ufunc.h"

namespace Pol
{
namespace Core
{
NoCastRegion::NoCastRegion( Clib::ConfigElem& elem, RegionId id )
    : Region( elem, id ), nocast_( elem.remove_bool( "nocast", false ) )
{
}

size_t NoCastRegion::estimateSize() const
{
  return base::estimateSize() + sizeof( bool ); /*nocast_*/
}

void read_nocast_zones()
{
  gamestate.nocastdef = new NoCastDef( "nocast" );
  read_region_data( *gamestate.nocastdef, "regions/nocast.cfg", "regions/regions.cfg",
                    "NoCastRegion Region" );
}

LightRegion::LightRegion( Clib::ConfigElem& elem, RegionId id )
    : Region( elem, id ), lightlevel( elem.remove_ushort( "LightLevel", 0 ) )
{
}

size_t LightRegion::estimateSize() const
{
  return base::estimateSize() + sizeof( unsigned ); /*lightlevel*/
}

void read_light_zones()
{
  gamestate.lightdef = new LightDef( "light" );
  read_region_data( *gamestate.lightdef, "regions/light.cfg", "regions/regions.cfg",
                    "LightRegion Region" );
}


WeatherRegion::WeatherRegion( Clib::ConfigElem& elem, RegionId id )
    : Region( elem, id ),
      weathertype( static_cast<u8>( elem.remove_ushort(
          "WeatherType", 255 ) ) ),  // dave changed 6/30/03, 255 is no weather, not 0
      severity( static_cast<u8>( elem.remove_ushort( "WeatherSeverity", 0 ) ) ),
      aux( static_cast<u8>( elem.remove_ushort( "WeatherAux", 0 ) ) ),
      lightoverride( elem.remove_int( "LightOverride", -1 ) )
{
}

size_t WeatherRegion::estimateSize() const
{
  return base::estimateSize() + 3 * sizeof( unsigned char ) /*weathertype severity aux*/
         + sizeof( int );                                   /*lightoverride*/
}

WeatherDef::WeatherDef( const char* name ) : RegionGroup<WeatherRegion>( name )
{
  for ( auto const& realm : gamestate.Realms )
  {
    unsigned int gridwidth = realm->width() / ZONE_SIZE;
    unsigned int gridheight = realm->height() / ZONE_SIZE;

    RegionId** zone = new RegionId*[gridwidth];

    for ( unsigned int i = 0; i < gridwidth; i++ )
    {
      zone[i] = new RegionId[gridheight];
      for ( unsigned int j = 0; j < gridheight; j++ )
      {
        zone[i][j] = 0;
      }
    }
    default_regionrealms.insert( std::make_pair( realm, zone ) );
  }
}

WeatherDef::~WeatherDef()
{
  for ( auto& realmregion : default_regionrealms )
  {
    unsigned int gridwidth = realmregion.first->width() / ZONE_SIZE;

    for ( unsigned int i = 0; i < gridwidth; i++ )
      delete[] realmregion.second[i];
    delete[] realmregion.second;
  }
}

size_t WeatherDef::estimateSize() const
{
  size_t size = RegionGroup<WeatherRegion>::estimateSize();

  for ( const auto& realm : default_regionrealms )
  {
    unsigned int gridwidth = realm.first->width() / ZONE_SIZE;
    size +=
        gridwidth * sizeof( RegionId ) + sizeof( Realms::Realm* ) + ( sizeof( void* ) * 3 + 1 ) / 2;
  }
  return size;
}

void WeatherDef::copy_default_regions()
{
  for ( auto& realmregion : regionrealms )
  {
    Realms::Realm* realm = realmregion.first;
    unsigned int gridwidth = realm->width() / ZONE_SIZE;
    unsigned int gridheight = realm->height() / ZONE_SIZE;

    for ( unsigned int i = 0; i < gridwidth; i++ )
    {
      for ( unsigned int j = 0; j < gridheight; j++ )
      {
        default_regionrealms[realm][i][j] = regionrealms[realm][i][j];
      }
    }
  }
}

bool WeatherDef::assign_zones_to_region( const char* regionname, Pos2d p1, Pos2d p2,
                                         Realms::Realm* realm )
{
  p1.crop( realm );
  p2.crop( realm );
  Pos2d zone1 = XyToZone( p1 );
  Pos2d zone2 = XyToZone( p2 );
  if ( regionname && regionname[0] )
  {
    Region* rgn = getregion_byname( regionname );
    if ( rgn == nullptr )
      return false;

    for ( auto zx = zone1.x(); zx <= zone2.x(); ++zx )
    {
      for ( auto zy = zone1.y(); zy <= zone2.y(); ++zy )
      {
        regionrealms[realm][zx][zy] = rgn->regionid();
      }
    }
  }
  else  // move 'em back to the default
  {
    for ( auto zx = zone1.x(); zx <= zone2.x(); ++zx )
    {
      for ( auto zy = zone1.y(); zy <= zone2.y(); ++zy )
      {
        regionrealms[realm][zx][zy] = default_regionrealms[realm][zx][zy];
      }
    }
  }
  update_all_weatherregions();
  return true;
}

void read_weather_zones()
{
  gamestate.weatherdef = new WeatherDef( "weather" );
  read_region_data( *gamestate.weatherdef, "regions/weather.cfg", "regions/regions.cfg",
                    "WeatherRegion Region" );

  gamestate.weatherdef->copy_default_regions();
}
}  // namespace Core
}  // namespace Pol
