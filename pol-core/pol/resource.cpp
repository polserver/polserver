/** @file
 *
 * @par History
 * - 2007/06/17 Shinigami: added config.world_data_path
 */


#include "resource.h"

#include "../bscript/berror.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/random.h"
#include "../clib/streamsaver.h"
#include "../plib/maptile.h"
#include "../plib/systemstate.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "item/itemdesc.h"
#include "polcfg.h"
#include "polclock.h"
#include "polsig.h"
#include "realms/realm.h"

namespace Pol
{
namespace Core
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


void ResourceRegion::read_data( Clib::ConfigElem& elem )
{
  units_ = elem.remove_ulong( "Units" );
}


/// [1] Harvest Difficulty
///
Bscript::BObjectImp* ResourceRegion::get_harvest_difficulty( xcoord x, ycoord y,
                                                             Realms::Realm* realm )
{
  (void)realm;
  unsigned int xy = ( x << 16 ) | y;
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

Bscript::BObjectImp* ResourceRegion::harvest_resource( xcoord x, ycoord y, int n, int m )
{
  unsigned int xy = ( x << 16 ) | y;
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

bool ResourceDef::findmarker( xcoord x, ycoord y, Realms::Realm* realm, unsigned int objtype )
{
  if ( !landtiles_.count( Items::getgraphic( objtype ) ) &&
       !tiles_.count( Items::getgraphic( objtype ) ) )
    return false;
  if ( realm->findstatic( static_cast<unsigned short>( x ), static_cast<unsigned short>( y ),
                          static_cast<unsigned short>( objtype ) ) )
  {
    return true;
  }

  // FIXME range can be bad
  Plib::MAPTILE_CELL cell =
      realm->getmaptile( static_cast<unsigned short>( x ), static_cast<unsigned short>( y ) );
  return ( cell.landtile == objtype );  // FIXME blech! objtype == landtile? eh? well broken anyway.
}


void ResourceDef::counttiles()
{
  unsigned int tilecount = 0;

  std::vector<Realms::Realm*>::iterator itr;
  for ( itr = gamestate.Realms.begin(); itr != gamestate.Realms.end(); ++itr )
  {
    for ( unsigned short x = 0; x < ( *itr )->width(); ++x )
    {
      for ( unsigned short y = 0; y < ( *itr )->height(); ++y )
      {
        Plib::MAPTILE_CELL cell = ( *itr )->getmaptile( x, y );

        if ( landtiles_.count( cell.landtile ) )
        {
          ++tilecount;
          ResourceRegion* rgn = getregion( x, y, *itr );
          if ( rgn )
            ++rgn->tilecount_;
        }
      }
    }
  }
  POLLOG_INFO << "Resource " << name() << ": " << tilecount << "\n";
  for ( unsigned i = 0; i < regions_.size(); ++i )
  {
    ResourceRegion* rrgn = static_cast<ResourceRegion*>( regions_[i] );
    POLLOG_INFO << "Region " << regions_[i]->name() << ": " << rrgn->tilecount_ << "\n";
  }
}

void ResourceDef::regenerate( time_t now )
{
  for ( unsigned i = 0; i < regions_.size(); ++i )
  {
    ResourceRegion* rrgn = static_cast<ResourceRegion*>( regions_[i] );
    rrgn->regenerate( now );
  }
}

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
  for ( ResourceDefs::iterator itr = gamestate.resourcedefs.begin(),
                               end = gamestate.resourcedefs.end();
        itr != end; ++itr )
  {
    ( *itr ).second->regenerate( now );
  }
  THREAD_CHECKPOINT( tasks, 799 );
}
void count_resource_tiles()
{
  for ( ResourceDefs::iterator itr = gamestate.resourcedefs.begin(),
                               end = gamestate.resourcedefs.end();
        itr != end; ++itr )
  {
    ( *itr ).second->counttiles();
  }
}

Bscript::BObjectImp* get_harvest_difficulty( const char* resource, xcoord x, ycoord y,
                                             Realms::Realm* realm, unsigned short marker )
{
  ResourceDefs::iterator itr = gamestate.resourcedefs.find( resource );
  if ( itr == gamestate.resourcedefs.end() )
    return new Bscript::BError( "No resource by that name" );

  ResourceDef* rd = ( *itr ).second;
  if ( !rd->findmarker( x, y, realm, marker ) )
    return new Bscript::BError( "No resource-bearing landmark there" );

  ResourceRegion* rgn = rd->getregion( x, y, realm );
  if ( rgn == NULL )
    return new Bscript::BError( "No resource region at that location" );

  return rgn->get_harvest_difficulty( x, y, realm );
}

Bscript::BObjectImp* harvest_resource( const char* resource, xcoord x, ycoord y,
                                       Realms::Realm* realm, int b, int n )
{
  ResourceDefs::iterator itr = gamestate.resourcedefs.find( resource );
  if ( itr == gamestate.resourcedefs.end() )
    return new Bscript::BError( "No resource by that name" );

  ResourceDef* rd = ( *itr ).second;

  /* assume GetHarvestDifficulty was already checked
      if (!rd->findmarker(x,y,marker))
      return new BError( "No resource-bearing landmark there" );
      */
  ResourceRegion* rgn = rd->getregion( x, y, realm );
  if ( rgn == NULL )
    return new Bscript::BError( "No resource region at that location" );

  return rgn->harvest_resource( x, y, b, n );
}

Bscript::BObjectImp* get_region_string( const char* resource, xcoord x, ycoord y,
                                        Realms::Realm* realm, const std::string& propname )
{
  ResourceDefs::iterator itr = gamestate.resourcedefs.find( resource );
  if ( itr == gamestate.resourcedefs.end() )
    return new Bscript::BError( "No resource by that name" );

  ResourceDef* rd = ( *itr ).second;

  ResourceRegion* rgn = rd->getregion( x, y, realm );
  if ( rgn == NULL )
    return new Bscript::BError( "No resource region at that location" );

  return rgn->get_region_string( propname );
}


ResourceDef* find_resource_def( const std::string& rname )
{
  ResourceDefs::iterator itr = gamestate.resourcedefs.find( rname );
  if ( itr == gamestate.resourcedefs.end() )
  {
    return NULL;
  }
  else
  {
    return ( *itr ).second;
  }
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
      INFO_PRINT << "File regions/resource.cfg not found, skipping.\n";
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
  if ( rd == NULL )
  {
    ERROR_PRINT << "Error reading RESOURCE.DAT: Unable to find resource type " << elem.rest()
                << "\n";
    throw std::runtime_error( "Data file error" );
  }

  rd->read_data( elem );
}
void read_region_data( Clib::ConfigElem& elem )
{
  ResourceDef* rd = find_resource_def( elem.rest() );
  if ( rd == NULL )
  {
    ERROR_PRINT << "Error reading RESOURCE.DAT: Unable to find resource type " << elem.rest()
                << "\n";
    throw std::runtime_error( "Data file error" );
  }
  std::string regionname = elem.remove_string( "Name" );
  ResourceRegion* rgn = rd->getregion( regionname );
  if ( rgn == NULL )
  {
    ERROR_PRINT << "Error reading RESOURCE.DAT: Unable to find region " << regionname
                << " in resource " << elem.rest() << "\n";
    throw std::runtime_error( "Data file error" );
  }
  rgn->read_data( elem );
}

void read_resources_dat()
{
  std::string resourcefile = Plib::systemstate.config.world_data_path + "resource.dat";

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
  sw() << "GlobalResourcePool " << name() << '\n'
       << "{" << '\n'
       << "\tUnits\t" << current_units_ << '\n'
       << "}" << '\n'
       << '\n';

  for ( unsigned i = 0; i < regions_.size(); ++i )
  {
    ResourceRegion* rrgn = static_cast<ResourceRegion*>( regions_[i] );
    rrgn->write( sw, name() );
  }
}

size_t ResourceDef::estimateSize() const
{
  size_t size =
      RegionGroup<ResourceRegion>::estimateSize() + sizeof( unsigned int ) /*initial_units_*/
      + sizeof( int )                                                      /*current_units_*/
      + 3 * sizeof( void* ) +
      landtiles_.size() * ( sizeof( unsigned short ) + 3 * sizeof( void* ) ) + 3 * sizeof( void* ) +
      tiles_.size() * ( sizeof( unsigned short ) + 3 * sizeof( void* ) );
  return size;
}


void ResourceRegion::write( Clib::StreamWriter& sw, const std::string& resource_name ) const
{
  sw() << "RegionalResourcePool " << resource_name << '\n'
       << "{" << '\n'
       << "\tName\t" << name_ << '\n'
       << "\tUnits\t" << units_ << '\n'
       << "#\t(regions/" << resource_name << ".cfg: Capacity is " << capacity_ << ")" << '\n'
       << "}" << '\n'
       << '\n';
  // sw.flush();
}

size_t ResourceRegion::estimateSize() const
{
  size_t size =
      Region::estimateSize() +
      5 * sizeof( unsigned int ) /*tilecount_ units_per_area_ seconds_per_regrow_ capacity_ units_*/
      + sizeof( time_t )         /*last_regen_*/
      + ( sizeof( unsigned int ) + sizeof( unsigned short ) + ( sizeof( void* ) * 3 + 1 ) / 2 ) *
            depletions_.size();
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
}
}
