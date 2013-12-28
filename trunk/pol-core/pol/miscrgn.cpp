/*
History
=======
2005/01/23 Shinigami: WeatherDef::Con-/Destructor & ::copy_default_regions - Tokuno MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)

Notes
=======

*/


#include "../clib/stl_inc.h"
#include "../clib/cfgelem.h"

#include "../plib/realm.h"

#include "region.h"
#include "miscrgn.h"
#include "ufunc.h"
#include "realms.h"
#include "uworld.h"
namespace Pol {
  namespace Core {
	NoCastRegion::NoCastRegion( Clib::ConfigElem& elem, RegionId id ) :
	  Region( elem, id ),
	  nocast_( elem.remove_bool( "nocast", false ) )
	{}

	NoCastDef* nocastdef;

	void read_nocast_zones()
	{
	  nocastdef = new NoCastDef( "nocast" );
	  read_region_data( *nocastdef,
						"regions/nocast.cfg",
						"regions/regions.cfg",
						"NoCastRegion Region" );
	}

    LightRegion::LightRegion( Clib::ConfigElem& elem, RegionId id ) :
	  Region( elem, id ),
	  lightlevel( elem.remove_ushort( "LightLevel", 0 ) )
	{}

	LightDef* lightdef;

	void read_light_zones()
	{
	  lightdef = new LightDef( "light" );
	  read_region_data( *lightdef,
						"regions/light.cfg",
						"regions/regions.cfg",
						"LightRegion Region" );
	}


    WeatherRegion::WeatherRegion( Clib::ConfigElem& elem, RegionId id ) :
	  Region( elem, id ),
	  weathertype( static_cast<u8>( elem.remove_ushort( "WeatherType", 255 ) ) ), //dave changed 6/30/03, 255 is no weather, not 0
	  severity( static_cast<u8>( elem.remove_ushort( "WeatherSeverity", 0 ) ) ),
	  aux( static_cast<u8>( elem.remove_ushort( "WeatherAux", 0 ) ) ),
	  lightoverride( elem.remove_int( "LightOverride", -1 ) )
	{}

	WeatherDef::WeatherDef( const char *name ) : RegionGroup<WeatherRegion>( name )
	{
      for ( auto const &realm : *Realms )
      {
        unsigned int gridwidth = realm->width() / WGRID_SIZE;
        unsigned int gridheight = realm->height() / WGRID_SIZE;

        // Tokuno-Fix
        if ( gridwidth * WGRID_SIZE < realm->width() )
          gridwidth++;
        if ( gridheight * WGRID_SIZE < realm->height() )
          gridheight++;

        RegionId** zone = new RegionId*[gridwidth];

        for ( unsigned int i = 0; i < gridwidth; i++ )
        {
          zone[i] = new RegionId[gridheight];
          for ( unsigned int j = 0; j < gridheight; j++ )
          {
            zone[i][j] = 0;
          }
        }
        default_regionrealms.insert( make_pair( realm, zone ) );
      }
	}

	WeatherDef::~WeatherDef()
	{
	  for ( auto &realmregion : default_regionrealms )
	  {
        unsigned int gridwidth = realmregion.first->width() / WGRID_SIZE;

		// Tokuno-Fix
        if ( gridwidth * WGRID_SIZE < realmregion.first->width() )
		  gridwidth++;

		for ( unsigned int i = 0; i < gridwidth; i++ )
          delete[] realmregion.second[i];
        delete[] realmregion.second;
	  }
	}

	void WeatherDef::copy_default_regions()
	{
	  //memcpy( &default_regionidx_, regionidx_, sizeof default_regionidx_ );
      for ( auto &realmregion : regionrealms )
	  {
        Plib::Realm* realm = realmregion.first;
		unsigned int gridwidth = realm->width() / WGRID_SIZE;
		unsigned int gridheight = realm->height() / WGRID_SIZE;

		// Tokuno-Fix
		if ( gridwidth * WGRID_SIZE < realm->width() )
		  gridwidth++;
		if ( gridheight * WGRID_SIZE < realm->height() )
		  gridheight++;

		//RegionId** zone = new RegionId*[gridwidth];

		for ( unsigned int i = 0; i < gridwidth; i++ )
		{
		  for ( unsigned int j = 0; j < gridheight; j++ )
		  {
			default_regionrealms[realm][i][j] = regionrealms[realm][i][j];
		  }
		}
	  }
	}

	bool WeatherDef::assign_zones_to_region(
	  const char* regionname,
	  unsigned short xwest, unsigned short ynorth,
	  unsigned short xeast, unsigned short ysouth,
	  Plib::Realm* realm )
	{
	  if ( xwest >= realm->width() ) xwest = static_cast<unsigned short>( realm->width() ) - 1;
	  if ( xeast >= realm->width() ) xeast = static_cast<unsigned short>( realm->width() ) - 1;
	  if ( ynorth >= realm->height() ) ynorth = static_cast<unsigned short>( realm->height() ) - 1;
	  if ( ysouth >= realm->height() ) ysouth = static_cast<unsigned short>( realm->height() ) - 1;


	  if ( regionname && regionname[0] )
	  {
		Region* rgn = getregion_byname( regionname );
		if ( rgn == NULL )
		  return false;

		unsigned zone_xwest, zone_ynorth, zone_xeast, zone_ysouth;
		XyToZone( xwest, ynorth, &zone_xwest, &zone_ynorth );
		XyToZone( xeast, ysouth, &zone_xeast, &zone_ysouth );
		unsigned zx, zy;
		for ( zx = zone_xwest; zx <= zone_xeast; ++zx )
		{
		  for ( zy = zone_ynorth; zy <= zone_ysouth; ++zy )
		  {
			regionrealms[realm][zx][zy] = rgn->regionid();
		  }
		}
	  }
	  else // move 'em back to the default
	  {
		unsigned zone_xwest, zone_ynorth, zone_xeast, zone_ysouth;
		XyToZone( xwest, ynorth, &zone_xwest, &zone_ynorth );
		XyToZone( xeast, ysouth, &zone_xeast, &zone_ysouth );
		unsigned zx, zy;
		for ( zx = zone_xwest; zx <= zone_xeast; ++zx )
		{
		  for ( zy = zone_ynorth; zy <= zone_ysouth; ++zy )
		  {
			regionrealms[realm][zx][zy] = default_regionrealms[realm][zx][zy];
		  }
		}
	  }
	  update_all_weatherregions();
	  return true;
	}

	WeatherDef* weatherdef;

	void read_weather_zones()
	{
	  weatherdef = new WeatherDef( "weather" );
	  read_region_data( *weatherdef,
						"regions/weather.cfg",
						"regions/regions.cfg",
						"WeatherRegion Region" );

	  weatherdef->copy_default_regions();
	}
  }
}
