/** @file
 *
 * @par History
 * - 2005/01/23 Shinigami: Realm::Con-/Destructor - Tokuno MapDimension doesn't fit blocks of 64x64
 * (WGRID_SIZE)
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of is_shadowrealm, baserealm and
 * shadowname
 */


#include "realm.h"

#include "../../plib/mapserver.h"
#include "../../plib/maptileserver.h"
#include "../../plib/realmdescriptor.h"
#include "../../plib/staticserver.h"
#include "../mobile/charactr.h"
#include "../poltype.h"
#include "../uworld.h"
#include "WorldChangeReasons.h"

namespace Pol
{
namespace Realms
{
Realm::Realm( const std::string& realm_name, const std::string& realm_path )
    : is_shadowrealm( false ),
      shadowid( 0 ),
      baserealm( nullptr ),
      _descriptor( Plib::RealmDescriptor::Load( realm_name, realm_path ) ),
      _mobile_count( 0 ),
      _offline_count( 0 ),
      _toplevel_item_count( 0 ),
      _multi_count( 0 ),
      _mapserver( Plib::MapServer::Create( _descriptor ) ),
      _staticserver( new Plib::StaticServer( _descriptor ) ),
      _maptileserver( new Plib::MapTileServer( _descriptor ) )
{
  size_t gridwidth = grid_width();
  size_t gridheight = grid_height();

  zone = new Core::Zone*[gridwidth];

  for ( size_t i = 0; i < gridwidth; i++ )
    zone[i] = new Core::Zone[gridheight];
}

Realm::Realm( const std::string& realm_name, Realm* realm )
    : is_shadowrealm( true ),
      shadowid( 0 ),
      baserealm( realm ),
      shadowname( realm_name ),
      _descriptor( realm->_descriptor ),
      _mobile_count( 0 ),
      _offline_count( 0 ),
      _toplevel_item_count( 0 ),
      _multi_count( 0 )
{
  size_t gridwidth = grid_width();
  size_t gridheight = grid_height();

  zone = new Core::Zone*[gridwidth];

  for ( size_t i = 0; i < gridwidth; i++ )
    zone[i] = new Core::Zone[gridheight];
}

Realm::~Realm()
{
  size_t gridwidth = grid_width();
  for ( size_t i = 0; i < gridwidth; i++ )
    delete[] zone[i];
  delete[] zone;
}

size_t Realm::sizeEstimate() const
{
  size_t size = sizeof( *this );
  size += shadowname.capacity();
  // zone **
  unsigned gridwidth = grid_width();
  unsigned gridheight = grid_height();

  for ( unsigned x = 0; x < gridwidth; ++x )
  {
    for ( unsigned y = 0; y < gridheight; ++y )
    {
      size += 3 * sizeof( void** ) + zone[x][y].characters.capacity() * sizeof( void* );
      size += 3 * sizeof( void** ) + zone[x][y].npcs.capacity() * sizeof( void* );
      size += 3 * sizeof( void** ) + zone[x][y].items.capacity() * sizeof( void* );
      size += 3 * sizeof( void** ) + zone[x][y].multis.capacity() * sizeof( void* );
    }
  }

  // estimated set footprint
  size +=
      3 * sizeof( void* ) + global_hulls.size() * ( sizeof( unsigned int ) + 3 * sizeof( void* ) );
  size += _descriptor.sizeEstimate() + ( ( !_mapserver ) ? 0 : _mapserver->sizeEstimate() ) +
          ( ( !_staticserver ) ? 0 : _staticserver->sizeEstimate() ) +
          ( ( !_maptileserver ) ? 0 : _maptileserver->sizeEstimate() );
  return size;
}

unsigned short Realm::grid_width() const
{
  return _descriptor.grid_width;
}
unsigned short Realm::grid_height() const
{
  return _descriptor.grid_height;
}

unsigned Realm::season() const
{
  return _descriptor.season;
}

bool Realm::valid( unsigned short x, unsigned short y, short z ) const
{
  return ( x < width() && y < height() && z >= Core::ZCOORD_MIN && z <= Core::ZCOORD_MAX );
}

const std::string Realm::name() const
{
  if ( is_shadowrealm )
    return shadowname;
  return _descriptor.name;
}

// This function will be called whenever:
//
//      - a npc is created,
//      - a npc is loaded from npcs.txt,
//      - a player character is created,
//      - a player character is loaded from pcs.txt,
//      - a player character connects,
//      - a player or npc gets moved from one realm to another
//
void Realm::add_mobile( const Mobile::Character& chr, WorldChangeReason reason )
{
  switch ( reason )
  {
  case WorldChangeReason::Moved:
    if ( !chr.logged_in() )
      ++_offline_count;
    break;

  case WorldChangeReason::PlayerLoad:
    ++_offline_count;
    break;

  case WorldChangeReason::PlayerEnter:
    --_offline_count;
    break;

  default:
    break;
  }

  // For some reason, characters are loaded with logged_in = true by default.
  // This only changes AFTER they are added here in read_data()...
  //
  // Of course, to make life harder, when PlayerEnter calls here the logged_in is still false. Yay!
  //
  if ( reason != WorldChangeReason::PlayerLoad &&
       ( reason == WorldChangeReason::PlayerEnter || chr.logged_in() ) )
    ++_mobile_count;
}

// This function will be called whenever:
//
//      - a npc is killed,
//      - a player character disconnects,
//      - a player character is deleted
//      - a player or npc gets moved from one realm to another
//
void Realm::remove_mobile( const Mobile::Character& chr, WorldChangeReason reason )
{
  switch ( reason )
  {
  case WorldChangeReason::PlayerExit:
    ++_offline_count;
    break;

  case WorldChangeReason::Moved:
  case WorldChangeReason::PlayerDeleted:
    if ( !chr.logged_in() )
    {
      --_offline_count;
    }
    break;

  default:
    break;
  }

  if ( chr.logged_in() )
    --_mobile_count;
}
}
}
