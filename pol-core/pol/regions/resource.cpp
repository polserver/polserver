/** @file
 *
 * @par History
 * - 2007/06/17 Shinigami: added config.world_data_path
 */


#include "resource.h"

#include "bscript/berror.h"
#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/random.h"
#include "clib/stlutil.h"
#include "clib/streamsaver.h"
#include "plib/maptile.h"
#include "plib/systemstate.h"

#include "globals/state.h"
#include "globals/uvars.h"
#include "item/itemdesc.h"
#include "polclock.h"
#include "polsig.h"
#include "realms/realm.h"


namespace Pol::Core
{
/// Resource Management
ResourceRegion::ResourceRegion( Clib::ConfigElem& elem, RegionId id )
    : Region( elem, id ),
      tilecount_( 0 ),

      units_per_area_( elem.remove_ulong( "UnitsPerArea" ) ),
      seconds_per_regrow_( elem.remove_ulong( "SecondsPerRegrow" ) ),
      last_regen_( poltime() ),

      capacity_( elem.remove_ulong( "Capacity" ) ),
      units_( 0 )

{
}

// TODO this variable gets read/write but never used??
// if we want to recover the region depletions_ would
// be the needed variable
void ResourceRegion::read_data( Clib::ConfigElem& elem )
{
  units_ = elem.remove_ulong( "Units" );
}


/// [1] Harvest Difficulty
///
Bscript::BObjectImp* ResourceRegion::get_harvest_difficulty( const Pos4d& pos )
{
  unsigned int xy = ( pos.x() << 16 ) | pos.y();
  Depletions::iterator itr = depletions_.find( xy );
  int units = units_per_area_;
  if ( itr != depletions_.end() )
  {
    units -= ( *itr ).second;
  }
  passert( units >= 0 );

  if ( units == 0 )
  {
    return new Bscript::BError( "Resource is depleted" );
  }

  unsigned int base_difficulty = 90 - ( units * 80 / units_per_area_ );
  unsigned int this_difficulty = Clib::random_int( ( base_difficulty * 2 ) - 1 );
  return new Bscript::BLong( this_difficulty );
}

Bscript::BObjectImp* ResourceRegion::harvest_resource( const Pos4d& pos, int n, int m )
{
  unsigned int xy = ( pos.x() << 16 ) | pos.y();
  Depletions::iterator itr = depletions_.find( xy );
  int avail_units = units_per_area_;
  if ( itr != depletions_.end() )
  {
    avail_units -= ( *itr ).second;
  }
  passert( avail_units >= 0 );
  int a = avail_units / n;
  if ( a > m )
    a = m;
  short na = static_cast<short>( n * a );
  if ( itr != depletions_.end() )
  {
    ( *itr ).second += na;
  }
  else
  {
    depletions_[xy] = na;
  }
  return new Bscript::BLong( na );
}

void ResourceRegion::regenerate( time_t now )
{
  unsigned int diff = static_cast<unsigned int>( now - last_regen_ );
  unsigned int n_regrow;
  if ( seconds_per_regrow_ )
  {
    n_regrow = diff / seconds_per_regrow_;
  }
  else
  {
    n_regrow = units_per_area_;
  }
  last_regen_ = now;
  //
  // a simple optimization.  If we'd regenerate everything,
  // just delete all depletions.
  //
  if ( n_regrow >= units_per_area_ )
  {
    depletions_.clear();
  }
  else
  {
    Depletions::iterator itr = depletions_.begin(), end = depletions_.end();
    while ( itr != end )
    {
      Depletions::iterator cur = itr;
      ++itr;
      if ( n_regrow >= ( *cur ).second )
        depletions_.erase( cur );
      else
        ( *cur ).second -= static_cast<unsigned short>( n_regrow );
    }
  }
}


ResourceDef::ResourceDef( const char* name )
    : RegionGroup<ResourceRegion>( name ), initial_units_( 0 ), current_units_( 0 )
{
}

void ResourceDef::read_config( Clib::ConfigElem& elem )
{
  initial_units_ = elem.remove_ulong( "InitialUnits" );

  // start current_units at initial_units in case there's no entry in
  // RESOURCE.DAT
  current_units_ = initial_units_;

  unsigned short tmp;
  while ( elem.remove_prop( "LandTile", &tmp ) )
  {
    landtiles_.insert( tmp );
  }
  while ( elem.remove_prop( "ObjType", &tmp ) )
  {
    tiles_.insert( tmp );
  }
}

void ResourceDef::read_data( Clib::ConfigElem& elem )
{
  current_units_ = static_cast<int>( elem.remove_ulong( "Units" ) );
}

bool ResourceDef::findmarker( const Pos4d& pos, unsigned int objtype )
{
  if ( !landtiles_.count( Items::getgraphic( objtype ) ) &&
       !tiles_.count( Items::getgraphic( objtype ) ) )
    return false;
  if ( pos.realm()->findstatic( pos.xy(), static_cast<unsigned short>( objtype ) ) )
  {
    return true;
  }

  // FIXME range can be bad
  Plib::MAPTILE_CELL cell = pos.realm()->getmaptile( pos.xy() );
  return ( cell.landtile == objtype );  // FIXME blech! objtype == landtile? eh? well broken anyway.
}


void ResourceDef::counttiles()
{
  unsigned int tilecount = 0;

  for ( const auto& realm : gamestate.Realms )
  {
    for ( const auto& p : realm->area() )
    {
      Plib::MAPTILE_CELL cell = realm->getmaptile( p );

      if ( landtiles_.count( cell.landtile ) )
      {
        ++tilecount;
        ResourceRegion* rgn = getregion( Pos4d( p, 0, realm ) );
        if ( rgn )
          ++rgn->tilecount_;
      }
    }
  }
  POLLOG_INFOLN( "Resource {}: {}", name(), tilecount );
  for ( auto& region : regions_ )
  {
    ResourceRegion* rrgn = static_cast<ResourceRegion*>( region );
    POLLOG_INFOLN( "Region {}: {}", region->name(), rrgn->tilecount_ );
  }
}

void ResourceDef::regenerate( time_t now )
{
  for ( auto& region : regions_ )
  {
    ResourceRegion* rrgn = static_cast<ResourceRegion*>( region );
    rrgn->regenerate( now );
  }
}

// TODO: why does it not check if enough units exists?
//  this gets called when an item with itemdesc.resource gets
//  created
void ResourceDef::consume( unsigned amount )
{
  current_units_ -= amount;
}

void ResourceDef::produce( unsigned amount )
{
  current_units_ += amount;
}

void regen_resources()
{
  THREAD_CHECKPOINT( tasks, 700 );
  time_t now = poltime();
  for ( auto& resourcedef : gamestate.resourcedefs )
  {
    resourcedef.second->regenerate( now );
  }
  THREAD_CHECKPOINT( tasks, 799 );
}
void count_resource_tiles()
{
  for ( auto& resourcedef : gamestate.resourcedefs )
  {
    resourcedef.second->counttiles();
  }
}

Bscript::BObjectImp* get_harvest_difficulty( const char* resource, const Pos4d& pos,
                                             unsigned short marker )
{
  ResourceDefs::iterator itr = gamestate.resourcedefs.find( resource );
  if ( itr == gamestate.resourcedefs.end() )
    return new Bscript::BError( "No resource by that name" );

  ResourceDef* rd = ( *itr ).second;
  if ( !rd->findmarker( pos, marker ) )
    return new Bscript::BError( "No resource-bearing landmark there" );

  ResourceRegion* rgn = rd->getregion( pos );
  if ( rgn == nullptr )
    return new Bscript::BError( "No resource region at that location" );

  return rgn->get_harvest_difficulty( pos );
}

Bscript::BObjectImp* harvest_resource( const char* resource, const Pos4d& pos, int b, int n )
{
  ResourceDefs::iterator itr = gamestate.resourcedefs.find( resource );
  if ( itr == gamestate.resourcedefs.end() )
    return new Bscript::BError( "No resource by that name" );

  ResourceDef* rd = ( *itr ).second;

  /* assume GetHarvestDifficulty was already checked
      if (!rd->findmarker(x,y,marker))
      return new BError( "No resource-bearing landmark there" );
      */
  ResourceRegion* rgn = rd->getregion( pos );
  if ( rgn == nullptr )
    return new Bscript::BError( "No resource region at that location" );

  return rgn->harvest_resource( pos, b, n );
}

Bscript::BObjectImp* get_region_string( const char* resource, const Pos4d& pos,
                                        const std::string& propname )
{
  ResourceDefs::iterator itr = gamestate.resourcedefs.find( resource );
  if ( itr == gamestate.resourcedefs.end() )
    return new Bscript::BError( "No resource by that name" );

  ResourceDef* rd = ( *itr ).second;

  ResourceRegion* rgn = rd->getregion( pos );
  if ( rgn == nullptr )
    return new Bscript::BError( "No resource region at that location" );

  return rgn->get_region_string( propname );
}


ResourceDef* find_resource_def( const std::string& rname )
{
  ResourceDefs::iterator itr = gamestate.resourcedefs.find( rname );
  if ( itr == gamestate.resourcedefs.end() )
  {
    return nullptr;
  }

  return ( *itr ).second;
}


void read_resource_cfg( const char* resource )
{
  std::string filename = std::string( "regions/" ) + resource + std::string( ".cfg" );
  Clib::ConfigFile cf( filename.c_str(), "GLOBAL REGION" );
  Clib::ConfigElem elem;

  std::unique_ptr<ResourceDef> rd( new ResourceDef( resource ) );

  while ( cf.read( elem ) )
  {
    if ( elem.type_is( "global" ) )
    {
      rd->read_config( elem );
    }
    else
    {
      rd->read_region( elem );
    }
  }

  gamestate.resourcedefs[resource] = rd.release();
}

void load_resource_cfg()
{
  if ( !Clib::FileExists( "regions/resource.cfg" ) )
  {
    if ( Plib::systemstate.config.loglevel > 1 )
      INFO_PRINTLN( "File regions/resource.cfg not found, skipping." );
    return;
  }

  Clib::ConfigFile cf( "regions/resource.cfg", "ResourceSystem" );
  Clib::ConfigElem elem;
  while ( cf.read( elem ) )
  {
    std::string resourcename;
    while ( elem.remove_prop( "ResourceType", &resourcename ) )
    {
      read_resource_cfg( resourcename.c_str() );
    }
  }
  if ( Plib::systemstate.config.count_resource_tiles )
    count_resource_tiles();
}


void read_global_data( Clib::ConfigElem& elem )
{
  ResourceDef* rd = find_resource_def( elem.rest() );
  if ( rd == nullptr )
  {
    ERROR_PRINTLN( "Error reading RESOURCE.TXT: Unable to find resource type {}", elem.rest() );
    throw std::runtime_error( "Data file error" );
  }

  rd->read_data( elem );
}
void read_region_data( Clib::ConfigElem& elem )
{
  ResourceDef* rd = find_resource_def( elem.rest() );
  if ( rd == nullptr )
  {
    ERROR_PRINTLN( "Error reading RESOURCE.TXT: Unable to find resource type {}", elem.rest() );
    throw std::runtime_error( "Data file error" );
  }
  std::string regionname = elem.remove_string( "Name" );
  ResourceRegion* rgn = rd->getregion( regionname );
  if ( rgn == nullptr )
  {
    ERROR_PRINTLN( "Error reading RESOURCE.TXT: Unable to find region {} in resource {}",
                   regionname, elem.rest() );
    throw std::runtime_error( "Data file error" );
  }
  rgn->read_data( elem );
}

void read_resources_dat()
{
  std::string resourcefile = Plib::systemstate.config.world_data_path + "resource.txt";

  if ( Clib::FileExists( resourcefile ) )
  {
    Clib::ConfigFile cf( resourcefile, "GLOBALRESOURCEPOOL REGIONALRESOURCEPOOL" );
    Clib::ConfigElem elem;
    while ( cf.read( elem ) )
    {
      if ( elem.type_is( "GlobalResourcePool" ) )
      {
        read_global_data( elem );
      }
      else
      {
        read_region_data( elem );
      }
    }
  }
}

void ResourceDef::write( Clib::StreamWriter& sw ) const
{
  sw.begin( "GlobalResourcePool", name() );
  sw.add( "Units", current_units_ );
  sw.end();

  for ( auto region : regions_ )
  {
    ResourceRegion* rrgn = static_cast<ResourceRegion*>( region );
    rrgn->write( sw, name() );
  }
}

size_t ResourceDef::estimateSize() const
{
  size_t size = RegionGroup<ResourceRegion>::estimateSize() +
                sizeof( unsigned int ) /*initial_units_*/
                + sizeof( int )        /*current_units_*/
                + Clib::memsize( landtiles_ ) + Clib::memsize( tiles_ );
  return size;
}


void ResourceRegion::write( Clib::StreamWriter& sw, const std::string& resource_name ) const
{
  sw.begin( "RegionalResourcePool", resource_name );
  sw.add( "Name", name_ );
  sw.add( "Units", units_ );
  sw.comment( "\t(regions/{}.cfg: Capacity is {})", resource_name, capacity_ );
  sw.end();
}

size_t ResourceRegion::estimateSize() const
{
  size_t size =
      Region::estimateSize() +
      5 * sizeof( unsigned int ) /*tilecount_ units_per_area_ seconds_per_regrow_ capacity_ units_*/
      + sizeof( time_t )         /*last_regen_*/
      + Clib::memsize( depletions_ );
  return size;
}

void write_resources_dat( Clib::StreamWriter& sw_resource )
{
  for ( const auto& resdef : gamestate.resourcedefs )
  {
    resdef.second->write( sw_resource );
  }
}

void clean_resources()
{
  for ( const auto& resdef : gamestate.resourcedefs )
  {
    delete resdef.second;
  }
  gamestate.resourcedefs.clear();
}
}  // namespace Pol::Core
