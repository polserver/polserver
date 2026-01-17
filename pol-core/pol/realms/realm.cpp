/** @file
 *
 * @par History
 * - 2005/01/23 Shinigami: Realm::Con-/Destructor - Tokuno MapDimension doesn't fit blocks of 64x64
 * (WGRID_SIZE)
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of is_shadowrealm, baserealm and
 * shadowname
 */

#include "realm.h"

#include "clib/stlutil.h"
#include "plib/mapserver.h"
#include "plib/maptileserver.h"
#include "plib/poltype.h"
#include "plib/realmdescriptor.h"
#include "plib/staticserver.h"

#include "mobile/charactr.h"
#include "realms/WorldChangeReasons.h"
#include "ufunc.h"
#include "uworld.h"


namespace Pol::Realms
{
Realm::Realm( const std::string& realm_name, const std::string& realm_path )
    : is_shadowrealm( false ),
      baserealm( nullptr ),
      has_decay( true ),
      _descriptor( Plib::RealmDescriptor::Load( realm_name, realm_path ) ),
      _mobile_count( 0 ),
      _offline_count( 0 ),
      _toplevel_item_count( 0 ),
      _multi_count( 0 ),
      _mapserver( Plib::MapServer::Create( _descriptor ) ),
      _staticserver( new Plib::StaticServer( _descriptor ) ),
      _maptileserver( new Plib::MapTileServer( _descriptor ) )
{
  _area = Core::Range2d( Core::Pos2d( 0, 0 ),
                         Core::Pos2d( _descriptor.width - 1, _descriptor.height - 1 ), nullptr );
  _gridarea = Core::Range2d( Core::Pos2d( 0, 0 ),
                             Core::Pos2d( _descriptor.grid_width - 1, _descriptor.grid_height - 1 ),
                             nullptr );
  size_t gridwidth = grid_width();
  size_t gridheight = grid_height();

  zone = new Core::Zone*[gridheight];

  for ( size_t i = 0; i < gridheight; i++ )
    zone[i] = new Core::Zone[gridwidth];
}

Realm::Realm( const std::string& realm_name, Realm* realm )
    : is_shadowrealm( true ),
      baserealm( realm ),
      shadowname( realm_name ),
      has_decay( true ),
      _descriptor( realm->_descriptor ),
      _mobile_count( 0 ),
      _offline_count( 0 ),
      _toplevel_item_count( 0 ),
      _multi_count( 0 )
{
  _area = Core::Range2d( Core::Pos2d( 0, 0 ),
                         Core::Pos2d( _descriptor.width - 1, _descriptor.height - 1 ), nullptr );
  _gridarea = Core::Range2d( Core::Pos2d( 0, 0 ),
                             Core::Pos2d( _descriptor.grid_width - 1, _descriptor.grid_height - 1 ),
                             nullptr );
  size_t gridwidth = grid_width();
  size_t gridheight = grid_height();

  zone = new Core::Zone*[gridheight];

  for ( size_t i = 0; i < gridheight; i++ )
    zone[i] = new Core::Zone[gridwidth];
}

Realm::~Realm()
{
  size_t gridheight = grid_height();
  for ( size_t i = 0; i < gridheight; i++ )
    delete[] zone[i];
  delete[] zone;
}

size_t Realm::sizeEstimate() const
{
  size_t size = sizeof( *this );
  size += shadowname.capacity();
  // zone **
  for ( const auto& p : gridarea() )
  {
    const auto& gzone = getzone_grid( p );
    size += Clib::memsize( gzone.characters ) + Clib::memsize( gzone.npcs ) +
            Clib::memsize( gzone.items ) + Clib::memsize( gzone.multis );
  }

  size += Clib::memsize( global_hulls );
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

bool Realm::valid( const Core::Pos2d& p ) const
{
  return _area.contains( p );
}
bool Realm::valid( const Core::Pos3d& p ) const
{
  return _area.contains( p.xy() );
}

const std::string Realm::name() const
{
  if ( is_shadowrealm )
    return shadowname;
  return _descriptor.name;
}

void Realm::notify_moved( Mobile::Character& whomoved )
{
  // When the movement is larger than 32 tiles, notify mobiles and items in the old location
  // TODO Pos magic 32 everywhere?
  // TODO its for npcs, with ex->area_size, NPC::update_range equal the area_size?
  if ( whomoved.distance_to( whomoved.lastpos ) > 32 )
  {
    Core::WorldIterator<Core::MobileFilter>::InRange(
        whomoved.lastpos, 32,
        [&]( Mobile::Character* chr ) { Mobile::NpcPropagateMove( chr, &whomoved ); } );

    Core::WorldIterator<Core::ItemFilter>::InRange(
        whomoved.lastpos, 32, [&]( Items::Item* item ) { item->inform_moved( &whomoved ); } );
  }

  // Inform nearby mobiles that a movement has been made.
  Core::WorldIterator<Core::MobileFilter>::InRange(
      &whomoved, 33,
      [&]( Mobile::Character* chr ) { Mobile::NpcPropagateMove( chr, &whomoved ); } );

  // the same for top-level items
  Core::WorldIterator<Core::ItemFilter>::InRange(
      &whomoved, 33, [&]( Items::Item* item ) { item->inform_moved( &whomoved ); } );
}

// The unhid character was already in the area and must have seen the other mobiles. So only notify
// the other mobiles in the region that a new one appeared.
void Realm::notify_unhid( Mobile::Character& whounhid )
{
  Core::WorldIterator<Core::NPCFilter>::InRange(
      &whounhid, 32,
      [&]( Mobile::Character* chr ) { Mobile::NpcPropagateEnteredArea( chr, &whounhid ); } );

  Core::WorldIterator<Core::ItemFilter>::InRange(
      &whounhid, 32, [&]( Items::Item* item ) { item->inform_enteredarea( &whounhid ); } );
}

// Resurrecting is just like unhiding
void Realm::notify_resurrected( Mobile::Character& whoressed )
{
  notify_unhid( whoressed );
}

void Realm::notify_entered( Mobile::Character& whoentered )
{
  Core::WorldIterator<Core::MobileFilter>::InRange(
      &whoentered, 32,
      [&]( Mobile::Character* chr )
      {
        Mobile::NpcPropagateEnteredArea( chr, &whoentered );
        Mobile::NpcPropagateEnteredArea( &whoentered,
                                         chr );  // Notify the one who entered this area about
                                                 // the mobiles that were already there
      } );

  // and notify the top-level items too
  Core::WorldIterator<Core::ItemFilter>::InRange(
      &whoentered, 32, [&]( Items::Item* item ) { item->inform_enteredarea( &whoentered ); } );
}

// Must be used right before a mobile leaves (before updating x and y)
void Realm::notify_left( Mobile::Character& wholeft )
{
  Core::WorldIterator<Core::MobileFilter>::InRange(
      &wholeft, 32,
      [&]( Mobile::Character* chr ) { Mobile::NpcPropagateLeftArea( chr, &wholeft ); } );

  Core::WorldIterator<Core::ItemFilter>::InRange(
      &wholeft, 32, [&]( Items::Item* item ) { item->inform_leftarea( &wholeft ); } );
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
}  // namespace Pol::Realms
