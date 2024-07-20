/** @file
 *
 * @par History
 * - 2005/01/23 Shinigami: WeatherDef::Con-/Destructor & WeatherDef::copy_default_regions - Tokuno
 * MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)
 */


#include "miscrgn.h"

#include <stddef.h>

#include "clib/cfgelem.h"
#include "clib/rawtypes.h"
#include "clib/stlutil.h"

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

    RegionId** zone = new RegionId*[gridheight];

    for ( unsigned int i = 0; i < gridheight; i++ )
    {
      zone[i] = new RegionId[gridwidth];
      for ( unsigned int j = 0; j < gridwidth; j++ )
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
    unsigned int gridheight = realmregion.first->height() / ZONE_SIZE;

    for ( unsigned int i = 0; i < gridheight; i++ )
      delete[] realmregion.second[i];
    delete[] realmregion.second;
  }
}

size_t WeatherDef::estimateSize() const
{
  size_t size = RegionGroup<WeatherRegion>::estimateSize();
  size += Clib::memsize( default_regionrealms );

  for ( const auto& realm : default_regionrealms )
  {
    unsigned int gridheight = realm.first->height() / ZONE_SIZE;
    size += gridheight * sizeof( RegionId );
  }
  return size;
}

void WeatherDef::copy_default_regions()
{
  for ( auto& realmregion : regionrealms )
  {
    Realms::Realm* realm = realmregion.first;
    Range2d area = Range2d( Pos2d( 0, 0 ), XyToZone( realm->area().se() ), nullptr );
    for ( const auto& p : area )
      default_regionrealms[realm][p.y()][p.x()] = regionrealms[realm][p.y()][p.x()];
  }
}

bool WeatherDef::assign_zones_to_region( const char* regionname, const Range2d& area,
                                         Realms::Realm* realm )
{
  Range2d zone_area( XyToZone( area.nw() ), XyToZone( area.se() ), nullptr );
  if ( regionname && regionname[0] )
  {
    Region* rgn = getregion_byname( regionname );
    if ( rgn == nullptr )
      return false;

    for ( const auto& p : zone_area )
      regionrealms[realm][p.y()][p.x()] = rgn->regionid();
  }
  else  // move 'em back to the default
  {
    for ( const auto& p : zone_area )
      regionrealms[realm][p.y()][p.x()] = default_regionrealms[realm][p.y()][p.x()];
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
