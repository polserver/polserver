/** @file
 *
 * @par History
 * - 2005/01/23 Shinigami: RegionGroupBase::Con-/Destructor - Tokuno MapDimension doesn't fit blocks
 * of 32x32 (ZONE_SIZE)
 */

#include "region.h"

#include <stddef.h>
#include <string>

#include "bscript/berror.h"
#include "bscript/impstr.h"
#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "clib/stlutil.h"
#include "plib/poltype.h"

#include "base/range.h"
#include "globals/uvars.h"
#include "proplist.h"
#include "realms/realm.h"
#include "realms/realms.h"
#include "zone.h"

namespace Pol
{
namespace Core
{
Region::Region( Clib::ConfigElem& elem, RegionId id )
    : name_( elem.rest() ), regionid_( id ), proplist_( Core::CPropProfiler::Type::REGION )
{
}

size_t Region::estimateSize() const
{
  return name_.capacity() + sizeof( RegionId ) + proplist_.estimatedSize();
}

void Region::read_custom_config( Clib::ConfigElem& elem )
{
  proplist_.readRemainingPropertiesAsStrings( elem );
}

Bscript::BObjectImp* Region::get_region_string( const std::string& propname )
{
  std::string propvalue;
  if ( !proplist_.getprop( propname, propvalue ) )
    return new Bscript::BError( "Property not found" );
  return new Bscript::String( propvalue );
}


RegionGroupBase::RegionGroupBase( const char* name ) : name_( name )
{
  for ( const auto& realm : gamestate.Realms )
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
    regionrealms.insert( std::make_pair( realm, zone ) );
  }
}
RegionGroupBase::~RegionGroupBase()
{
  for ( auto& realm : regionrealms )
  {
    unsigned int gridheight = realm.first->height() / ZONE_SIZE;

    // Tokuno-Fix removed Turley, 2009/09/08 (for ZONE_SIZE 4 not needed)
    /*if (gridwidth * ZONE_SIZE < itr->first->width())
      gridwidth++;*/

    for ( unsigned int i = 0; i < gridheight; i++ )
      delete[] realm.second[i];
    delete[] realm.second;
  }

  // cleans the regions_ vector...
  for ( auto& region : regions_ )
  {
    delete region;
  }
  regions_.clear();
}

void RegionGroupBase::paint_zones( Clib::ConfigElem& elem, RegionId ridx )
{
  std::string zonestr, strrealm;

  strrealm = elem.remove_string( "Realm", "britannia" );
  Realms::Realm* realm = find_realm( strrealm );
  if ( !realm )
    elem.throw_error( "Realm not found" );
  while ( elem.remove_prop( "Range", &zonestr ) )
  {
    unsigned short xwest, ynorth, xeast, ysouth;
    ISTRINGSTREAM is( zonestr );
    if ( is >> xwest >> ynorth >> xeast >> ysouth )
    {
      if ( xeast >= realm->width() || ysouth >= realm->height() )
      {
        elem.throw_error( "Zone range is out of bounds for realm" );
      }

      Range2d area( XyToZone( Pos2d( xwest, ynorth ) ), XyToZone( Pos2d( xeast, ysouth ) ),
                    nullptr );
      for ( const auto& itr : area )
        regionrealms[realm][itr.y()][itr.x()] = ridx;
    }
    else
    {
      elem.throw_error( "Poorly formed zone range: " + zonestr );
    }
  }
}

RegionId RegionGroupBase::getregionid( const Pos4d& pos )
{
  Pos2d zonep = XyToZone( pos.xy() );
  RegionId** regiongrp;
  if ( pos.realm()->is_shadowrealm )
    regiongrp = regionrealms[pos.realm()->baserealm];
  else
    regiongrp = regionrealms[pos.realm()];
  return regiongrp[zonep.y()][zonep.x()];
}

Region* RegionGroupBase::getregion_byname( const std::string& regionname )
{
  RegionsByName::iterator itr = regions_byname_.find( regionname );
  if ( itr == regions_byname_.end() )
    return nullptr;
  return ( *itr ).second;
}

Region* RegionGroupBase::getregion_byloc( const Pos4d& pos )
{
  RegionId ridx = getregionid( pos );

  // dave 12-22 return null if no regions, don't throw
  std::vector<Region*>::iterator itr = regions_.begin();
  if ( ( itr += ridx ) >= regions_.end() )
    return nullptr;
  return regions_[ridx];
}

void RegionGroupBase::read_region( Clib::ConfigElem& elem )
{
  Region* rgn = create_region( elem, static_cast<RegionId>( regions_.size() ) );
  regions_.push_back( rgn );
  regions_byname_.insert( RegionsByName::value_type( elem.rest(), rgn ) );

  paint_zones( elem, static_cast<RegionId>( regions_.size() - 1 ) );
  rgn->read_custom_config( elem );
}

void RegionGroupBase::create_bgnd_region( Clib::ConfigElem& elem )
{
  Region* rgn = create_region( elem, static_cast<RegionId>( regions_.size() ) );

  regions_.push_back( rgn );
  regions_byname_.insert( RegionsByName::value_type( "_background_", rgn ) );
  rgn->read_custom_config( elem );
}

size_t RegionGroupBase::estimateSize() const
{
  size_t size = 0;
  for ( const auto& region : regions_ )
  {
    size += region->estimateSize();
  }
  size += Clib::memsize( regionrealms );
  for ( const auto& realm : regionrealms )
  {
    if ( realm.first == nullptr )
      continue;
    unsigned int gridheight = realm.first->height() / ZONE_SIZE;
    size += gridheight * sizeof( RegionId );
  }
  size += name_.capacity();
  size += Clib::memsize( regions_byname_ );
  return size;
}

void read_region_data( RegionGroupBase& grp, const char* preferred_filename,
                       const char* other_filename, const char* tags_expected )
{
  const char* filename;
  if ( Clib::FileExists( preferred_filename ) )
    filename = preferred_filename;
  else
    filename = other_filename;

  if ( !Clib::FileExists( filename ) )
    return;
  Clib::ConfigFile cf( filename, tags_expected );
  Clib::ConfigElem elem;

  grp.create_bgnd_region( elem );

  while ( cf.read( elem ) )
  {
    grp.read_region( elem );
  }
}
}  // namespace Core
}  // namespace Pol
