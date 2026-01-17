/** @file
 *
 * @par History
 * - 2005/04/02 Shinigami: move_offline_mobiles - added realm param
 * - 2005/08/22 Shinigami: do_tellmoves - bugfix - sometimes we've destroyed objects because of
 * control scripts
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: cassert removed
 *                         STLport-5.2.1 fix: boat_components changed little bit
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 * - 2009/12/02 Turley:    added 0xf3 packet - Tomi
 * - 2011/11/12 Tomi:      added extobj.tillerman, extobj.port_plank, extobj.starboard_plank and
 * extobj.hold
 * - 2011/12/13 Tomi:      added support for new boats
 */

#include "boat.h"

#include <exception>
#include <string>

#include "../../bscript/berror.h"
#include "../../bscript/executor.h"
#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/clib.h"
#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/streamsaver.h"
#include "../../plib/systemstate.h"
#include "../../plib/tiles.h"
#include "../../plib/uconst.h"
#include "../../plib/ustruct.h"
#include "../containr.h"
#include "../extobj.h"
#include "../fnsearch.h"
#include "../globals/object_storage.h"
#include "../globals/settings.h"
#include "../globals/uvars.h"
#include "../item/item.h"
#include "../item/itemdesc.h"
#include "../mkscrobj.h"
#include "../mobile/charactr.h"
#include "../module/uomod.h"
#include "../network/client.h"
#include "../network/packethelper.h"
#include "../network/packets.h"
#include "../network/pktdef.h"
#include "../polvar.h"
#include "../realms/realm.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../syshookscript.h"
#include "../ufunc.h"
#include "../uobject.h"
#include "../uworld.h"
#include "boatcomp.h"
#include "multi.h"
#include "multidef.h"


namespace Pol::Multi
{
// #define DEBUG_BOATS

std::vector<Network::Client*> boat_sent_to;

BoatShape::ComponentShape::ComponentShape( const std::string& str, unsigned char type )
{
  altgraphic = 0;
  objtype = get_component_objtype( type );
  ISTRINGSTREAM is( str );
  std::string tmp;
  if ( is >> tmp )
  {
    graphic = static_cast<unsigned short>( strtoul( tmp.c_str(), nullptr, 0 ) );
    if ( graphic )
    {
      s16 xd, yd;
      if ( is >> xd >> yd )
      {
        delta = Core::Vec3d( xd, yd, 0 );
        s16 zd;
        if ( is >> zd )
          delta.z( zd );
        return;
      }
    }
  }

  ERROR_PRINTLN( "Boat component definition '{}' is poorly formed.", str );
  throw std::runtime_error( "Poorly formed boat.cfg component definition" );
}

BoatShape::ComponentShape::ComponentShape( const std::string& str, const std::string& altstr,
                                           unsigned char type )
    : ComponentShape( str, type )
{
  ISTRINGSTREAM altis( altstr );
  std::string alttmp;
  if ( altis >> alttmp )
  {
    altgraphic = static_cast<unsigned short>( strtoul( alttmp.c_str(), nullptr, 0 ) );
    return;
  }
  ERROR_PRINTLN( "Boat component definition '{}' is poorly formed.", str );
  throw std::runtime_error( "Poorly formed boat.cfg component definition" );
}


BoatShape::BoatShape( Clib::ConfigElem& elem )
{
  std::string tmp_str;

  while ( elem.remove_prop( "Tillerman", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, COMPONENT_TILLERMAN );
  while ( elem.remove_prop( "PortGangplankRetracted", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, elem.remove_string( "PortGangplankExtended" ),
                                  COMPONENT_PORT_PLANK );
  while ( elem.remove_prop( "StarboardGangplankRetracted", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, elem.remove_string( "StarboardGangplankExtended" ),
                                  COMPONENT_STARBOARD_PLANK );
  while ( elem.remove_prop( "Hold", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, COMPONENT_HOLD );
  while ( elem.remove_prop( "Rope", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, COMPONENT_ROPE );
  while ( elem.remove_prop( "Wheel", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, COMPONENT_WHEEL );
  while ( elem.remove_prop( "Hull", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, COMPONENT_HULL );
  while ( elem.remove_prop( "Tiller", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, COMPONENT_TILLER );
  while ( elem.remove_prop( "Rudder", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, COMPONENT_RUDDER );
  while ( elem.remove_prop( "Sails", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, COMPONENT_SAILS );
  while ( elem.remove_prop( "Storage", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, COMPONENT_STORAGE );
  while ( elem.remove_prop( "Weaponslot", &tmp_str ) )
    Componentshapes.emplace_back( tmp_str, COMPONENT_WEAPONSLOT );
}

bool BoatShape::objtype_is_component( unsigned int objtype )
{
  if ( objtype == Core::settingsManager.extobj.tillerman )
    return true;
  if ( objtype == Core::settingsManager.extobj.port_plank )
    return true;
  else if ( objtype == Core::settingsManager.extobj.starboard_plank )
    return true;
  else if ( objtype == Core::settingsManager.extobj.hold )
    return true;
  else if ( objtype == Core::settingsManager.extobj.rope )
    return true;
  else if ( objtype == Core::settingsManager.extobj.wheel )
    return true;
  else if ( objtype == Core::settingsManager.extobj.hull )
    return true;
  else if ( objtype == Core::settingsManager.extobj.tiller )
    return true;
  else if ( objtype == Core::settingsManager.extobj.rudder )
    return true;
  else if ( objtype == Core::settingsManager.extobj.sails )
    return true;
  else if ( objtype == Core::settingsManager.extobj.storage )
    return true;
  else if ( objtype == Core::settingsManager.extobj.weaponslot )
    return true;
  else
    return false;
}

size_t BoatShape::estimateSize() const
{
  return Clib::memsize( Componentshapes );
}

unsigned int get_component_objtype( unsigned char type )
{
  switch ( type )
  {
  case COMPONENT_TILLERMAN:
    return Core::settingsManager.extobj.tillerman;
  case COMPONENT_PORT_PLANK:
    return Core::settingsManager.extobj.port_plank;
  case COMPONENT_STARBOARD_PLANK:
    return Core::settingsManager.extobj.starboard_plank;
  case COMPONENT_HOLD:
    return Core::settingsManager.extobj.hold;
  case COMPONENT_ROPE:
    return Core::settingsManager.extobj.rope;
  case COMPONENT_WHEEL:
    return Core::settingsManager.extobj.wheel;
  case COMPONENT_HULL:
    return Core::settingsManager.extobj.hull;
  case COMPONENT_TILLER:
    return Core::settingsManager.extobj.tiller;
  case COMPONENT_RUDDER:
    return Core::settingsManager.extobj.rudder;
  case COMPONENT_SAILS:
    return Core::settingsManager.extobj.sails;
  case COMPONENT_STORAGE:
    return Core::settingsManager.extobj.storage;
  case COMPONENT_WEAPONSLOT:
    return Core::settingsManager.extobj.weaponslot;
  default:
    return 0;
  }
}

void read_boat_cfg()
{
  Clib::ConfigFile cf( "config/boats.cfg", "Boat" );
  Clib::ConfigElem elem;
  while ( cf.read( elem ) )
  {
    unsigned short multiid = elem.remove_ushort( "MultiID" );
    try
    {
      Core::gamestate.boatshapes[multiid] = new BoatShape( elem );
    }
    catch ( std::exception& )
    {
      ERROR_PRINTLN( "Error occurred reading definition for boat {:#x}", multiid );
      throw;
    }
  }
}

void clean_boatshapes()
{
  Core::BoatShapes::iterator iter = Core::gamestate.boatshapes.begin();
  for ( ; iter != Core::gamestate.boatshapes.end(); ++iter )
  {
    if ( iter->second != nullptr )
      delete iter->second;
    iter->second = nullptr;
  }
  Core::gamestate.boatshapes.clear();
}

bool BoatShapeExists( u16 multiid )
{
  return Core::gamestate.boatshapes.count( multiid ) != 0;
}

void UBoat::send_smooth_move( Network::Client* client, Core::UFACING move_dir, u8 speed,
                              bool relative ) const
{
  Network::PktHelper::PacketOut<Network::PktOut_F6> msg;

  Core::UFACING b_facing = boat_facing();
  if ( relative == false )
    move_dir = static_cast<Core::UFACING>( ( b_facing + move_dir ) * 7 );

  msg->offset += 2;  // Length
  msg->Write<u32>( serial_ext );
  msg->Write<u8>( speed );

  msg->Write<u8>( move_dir );
  msg->Write<u8>( b_facing );

  msg->WriteFlipped<u16>( x() );
  msg->WriteFlipped<u16>( y() );
  msg->WriteFlipped<s16>( z() );

  // 0xe000 encoding room, huffman can take more space then the maximum of 0xffff (10% + extra
  // margin)
  const u16 max_count = ( 0xe000 - 18 ) / 10;
  u16 object_count = 0;
  u16 len_offset = msg->offset;
  msg->offset += 2;  // Length
  for ( auto& component : Components )
  {
    if ( object_count >= max_count )
    {
      POLLOG_INFOLN( "Boat {:#x} at {} with {} items is too full - truncating movement packet",
                     serial, pos(), travellers_.size() );
      break;
    }
    if ( component == nullptr || component->orphan() )
      continue;

    msg->Write<u32>( component->serial_ext );
    msg->WriteFlipped<u16>( component->x() );
    msg->WriteFlipped<u16>( component->y() );
    msg->WriteFlipped<s16>( component->z() );
    ++object_count;
  }
  for ( auto& travellerRef : travellers_ )
  {
    if ( object_count >= max_count )
    {
      POLLOG_INFOLN( "Boat {:#x} at {} with {} items is too full - truncating movement packet",
                     serial, pos(), travellers_.size() );
      break;
    }
    UObject* obj = travellerRef.get();

    if ( obj->orphan() )
      continue;
    if ( obj->ismobile() )
    {
      auto* chr = static_cast<Mobile::Character*>( obj );
      if ( !client->chr->is_visible_to_me( chr, /*check_range*/ false ) )
        continue;
    }
    else
    {
      auto* item = static_cast<Items::Item*>( obj );
      if ( item->invisible() && !client->chr->can_seeinvisitems() )
        continue;
    }
    msg->Write<u32>( obj->serial_ext );
    msg->WriteFlipped<u16>( obj->x() );
    msg->WriteFlipped<u16>( obj->y() );
    msg->WriteFlipped<s16>( obj->z() );
    ++object_count;
  }
  u16 len = msg->offset;
  msg->offset = len_offset;
  msg->WriteFlipped<u16>( object_count );
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  msg.Send( client, len );
}

void UBoat::send_display_boat( Network::Client* client )
{
  Network::PktHelper::PacketOut<Network::PktOut_F7> msg;

  msg->offset += 2;  // Length

  // Send_display_boat is only called for CLIENTTYPE_7090, so each 0xF3 is 26 bytes here
  // 0xe000 encoding room, huffman can take more space then the maximum of 0xffff (10% + extra
  // margin)
  const u16 max_count = ( 0xe000 - 5 ) / 26;
  u16 object_count = 1;  // Add 1 for the boat aswell
  u16 len_offset = msg->offset;
  msg->offset += 2;  // Length

  // Build boat part
  msg->Write<u8>( 0xF3u );
  msg->WriteFlipped<u16>( 0x1u );
  msg->Write<u8>( 0x2u );  // MultiData flag
  msg->Write<u32>( this->serial_ext );
  msg->WriteFlipped<u16>( this->multidef().multiid );
  msg->offset++;                   // ID offset, TODO CHECK IF NEED THESE
  msg->WriteFlipped<u16>( 0x1u );  // Amount
  msg->WriteFlipped<u16>( 0x1u );  // Amount
  msg->WriteFlipped<u16>( this->x() );
  msg->WriteFlipped<u16>( this->y() );
  msg->Write<s8>( this->z() );
  msg->offset++;  // facing 0 for multis
  msg->WriteFlipped<u16>( this->color );
  msg->offset++;     // flags 0 for multis
  msg->offset += 2;  // HSA access flags, TODO find out what these are for and implement it

  for ( auto& component : Components )
  {
    if ( object_count >= max_count )
    {
      POLLOG_INFOLN( "Boat {:#x} at {} with {} items is too full - truncating display boat packet",
                     serial, pos(), travellers_.size() );
      break;
    }
    if ( component == nullptr || component->orphan() )
      continue;
    msg->Write<u8>( 0xF3u );
    msg->WriteFlipped<u16>( 0x1u );
    msg->Write<u8>( 0x0u );  // ItemData flag
    msg->Write<u32>( component->serial_ext );
    msg->WriteFlipped<u16>( component->graphic );
    msg->offset++;  // ID offset, TODO CHECK IF NEED THESE
    msg->WriteFlipped<u16>( component->get_senditem_amount() );  // Amount
    msg->WriteFlipped<u16>( component->get_senditem_amount() );  // Amount
    msg->WriteFlipped<u16>( component->x() );
    msg->WriteFlipped<u16>( component->y() );
    msg->Write<s8>( component->z() );
    msg->Write<u8>( component->facing );
    msg->WriteFlipped<u16>( component->color );
    msg->offset++;     // FLAGS, no flags for components
    msg->offset += 2;  // HSA access flags, TODO find out what these are for and implement it
    ++object_count;
  }
  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    if ( obj->orphan() )
      continue;
    if ( object_count >= max_count )
    {
      POLLOG_INFOLN( "Boat {:#x} at {} with {} items is too full - truncating display boat packet",
                     serial, pos(), travellers_.size() );
      break;
    }
    u8 flags = 0;
    if ( obj->ismobile() )
    {
      auto* chr = static_cast<Mobile::Character*>( obj );
      if ( !client->chr->is_visible_to_me( chr, /*check_range*/ false ) )
        continue;
    }
    else
    {
      auto* item = static_cast<Items::Item*>( obj );
      if ( item->invisible() && !client->chr->can_seeinvisitems() )
      {
        send_remove_object( client, item );
        continue;
      }
      if ( client->chr->can_move( item ) )
        flags |= ITEM_FLAG_FORCE_MOVABLE;
    }

    msg->Write<u8>( 0xF3u );
    msg->WriteFlipped<u16>( 0x1u );

    if ( obj->ismobile() )
      msg->Write<u8>( 0x1u );  // CharData flag
    else
      msg->Write<u8>( 0x0u );  // ItemData flag

    msg->Write<u32>( obj->serial_ext );
    msg->WriteFlipped<u16>( obj->graphic );
    msg->offset++;  // ID offset, TODO CHECK IF NEED THESE

    if ( obj->ismobile() )
    {
      msg->WriteFlipped<u16>( 0x1u );  // Amount
      msg->WriteFlipped<u16>( 0x1u );  // Amount
    }
    else
    {
      Items::Item* item = static_cast<Items::Item*>( obj );
      msg->WriteFlipped<u16>( item->get_senditem_amount() );  // Amount
      msg->WriteFlipped<u16>( item->get_senditem_amount() );  // Amount
    }

    msg->WriteFlipped<u16>( obj->x() );
    msg->WriteFlipped<u16>( obj->y() );
    msg->Write<s8>( obj->z() );
    msg->Write<u8>( obj->facing );
    msg->WriteFlipped<u16>( obj->color );

    msg->Write<u8>( flags );  // FLAGS
    msg->offset += 2;         // HSA access flags, TODO find out what these are for and implement it
    ++object_count;
  }
  u16 len = msg->offset;
  msg->offset = len_offset;
  msg->WriteFlipped<u16>( object_count );
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  msg.Send( client, len );
}

void UBoat::send_boat_newly_inrange( Network::Client* client )
{
  Network::PktHelper::PacketOut<Network::PktOut_F3> msg;
  msg->WriteFlipped<u16>( 0x1u );
  msg->Write<u8>( 0x02u );
  msg->Write<u32>( serial_ext );
  msg->WriteFlipped<u16>( multidef().multiid );
  msg->offset++;                   // 0;
  msg->WriteFlipped<u16>( 0x1u );  // amount
  msg->WriteFlipped<u16>( 0x1u );  // amount2
  msg->WriteFlipped<u16>( x() );
  msg->WriteFlipped<u16>( y() );
  msg->Write<s8>( z() );
  msg->offset++;                    // u8 facing
  msg->WriteFlipped<u16>( color );  // u16 color
  msg->offset += 3;                 // u8 flags + u16 HSA access flags

  msg.Send( client );

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    if ( !obj->orphan() )
    {
      if ( obj->ismobile() )
      {
        Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
        send_owncreate( client, chr );
      }
      else
      {
        Items::Item* item = static_cast<Items::Item*>( obj );
        send_item( client, item );
      }
    }
  }

  for ( auto& component : Components )
  {
    if ( component != nullptr && !component->orphan() )
      send_item( client, component.get() );
  }
}

void UBoat::send_display_boat_to_inrange( const std::optional<Core::Pos4d>& oldpos )
{
  Core::WorldIterator<Core::OnlinePlayerFilter>::InMaxVisualRange(
      this,
      [&]( Mobile::Character* zonechr )
      {
        Network::Client* client = zonechr->client;
        if ( !zonechr->in_visual_range( this ) )
          return;

        if ( client->ClientType & Network::CLIENTTYPE_7090 )
          send_display_boat( client );
        else if ( client->ClientType & Network::CLIENTTYPE_7000 )
          send_boat( client );
        else
          send_boat_old( client );
      } );

  if ( oldpos )
  {
    Core::WorldIterator<Core::OnlinePlayerFilter>::InMaxVisualRange(
        oldpos.value(),
        [&]( Mobile::Character* zonechr )
        {
          if ( !zonechr->in_visual_range( this, oldpos.value() ) )
            return;
          if ( !zonechr->in_visual_range( this ) )  // send remove to chrs only seeing the old loc
            send_remove_boat( zonechr->client );
        } );
  }
}

void UBoat::send_boat( Network::Client* client )
{
  // Client >= 7.0.0.0 ( SA )
  Network::PktHelper::PacketOut<Network::PktOut_F3> msg2;
  msg2->WriteFlipped<u16>( 0x1u );
  msg2->Write<u8>( 0x02u );
  msg2->Write<u32>( this->serial_ext );
  msg2->WriteFlipped<u16>( this->multidef().multiid );
  msg2->offset++;                   // 0;
  msg2->WriteFlipped<u16>( 0x1u );  // amount
  msg2->WriteFlipped<u16>( 0x1u );  // amount2
  msg2->WriteFlipped<u16>( this->x() );
  msg2->WriteFlipped<u16>( this->y() );
  msg2->Write<s8>( this->z() );
  msg2->offset++;                          // u8 facing
  msg2->WriteFlipped<u16>( this->color );  // u16 color
  msg2->offset++;                          // u8 flags

  msg2.Send( client );
}

void UBoat::send_boat_old( Network::Client* client )
{
  Network::PktHelper::PacketOut<Network::PktOut_1A> msg;
  msg->offset += 2;
  u16 b_graphic = this->multidef().multiid | 0x4000;
  msg->Write<u32>( this->serial_ext );
  msg->WriteFlipped<u16>( b_graphic );
  msg->WriteFlipped<u16>( this->x() );
  msg->WriteFlipped<u16>( this->y() );
  msg->Write<s8>( this->z() );
  u16 len1A = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len1A );

  client->pause();
  msg.Send( client, len1A );
  boat_sent_to.push_back( client );
}

void UBoat::send_remove_boat( Network::Client* client )
{
  send_remove_object( client, this );
}

void unpause_paused()
{
  for ( Network::Client* client : boat_sent_to )
  {
    client->restart();
  }
  boat_sent_to.clear();
}


UBoat::UBoat( const Items::ItemDesc& descriptor )
    : UMulti( descriptor ),
      tillerman( nullptr ),
      portplank( nullptr ),
      starboardplank( nullptr ),
      hold( nullptr ),
      mountpiece( nullptr )
{
  passert( Core::gamestate.boatshapes.count( multiid_ ) != 0 );
}

UBoat* UBoat::as_boat()
{
  return this;
}

bool UBoat::objtype_passable( unsigned short objtype )
{
  if ( Core::settingsManager.ssopt.boat_sails_collide )
    return false;

  auto uoflags = Plib::systemstate.tile[objtype].uoflags;
  bool passable = ( uoflags & Plib::USTRUCT_TILE::FLAG_FOLIAGE ) &&
                  ( ~uoflags & Plib::USTRUCT_TILE::FLAG_BLOCKING );
  return passable;
}

/* boats are a bit strange - they can move from side to side, forwards and
   backwards, without turning.
   */
void UBoat::regself()
{
  const MultiDef& md = multidef();
  for ( const auto& ele : md.hull )
  {
    if ( objtype_passable( ele->objtype ) )
      continue;

    Core::Pos2d hullpos = pos2d() + ele->relpos.xy();

    unsigned int gh = realm()->encode_global_hull( hullpos );
    realm()->global_hulls.insert( gh );
  }
}

void UBoat::unregself()
{
  const MultiDef& md = multidef();
  for ( const auto& ele : md.hull )
  {
    if ( objtype_passable( ele->objtype ) )
      continue;

    Core::Pos2d hullpos = pos2d() + ele->relpos.xy();

    unsigned int gh = realm()->encode_global_hull( hullpos );
    realm()->global_hulls.erase( gh );
  }
}

bool UBoat::can_fit_at_location( const MultiDef& md, const Core::Pos4d& desired_pos )
{
  if ( !desired_pos.can_move_to( md.minrxyz.xy() ) || !desired_pos.can_move_to( md.maxrxyz.xy() ) )
  {
#ifdef DEBUG_BOATS
    INFO_PRINTLN( "Location {} impassable, location is off the map", desired_pos );
#endif
    return false;
  }
  return true;
}

// navigable: Can the ship sit here?  ie is every point on the hull on water,and not blocked?
bool UBoat::navigable( const MultiDef& md, const Core::Pos4d& desired_pos )
{
  if ( !can_fit_at_location( md, desired_pos ) )
    return false;

  /* Test the external hull to make sure it's on water */

  for ( const auto& ele : md.hull )
  {
    Core::Pos3d hullpos = desired_pos.xyz() + ele->relpos;
#ifdef DEBUG_BOATS
    INFO_PRINT( "[{}]", hullpos );
#endif

    if ( objtype_passable( ele->objtype ) )
      continue;

    /*
     * See if any other ship hulls occupy this space
     */
    unsigned int gh = desired_pos.realm()->encode_global_hull( hullpos.xy() );
    if ( desired_pos.realm()->global_hulls.count( gh ) )  // already a boat there
    {
#ifdef DEBUG_BOATS
      INFO_PRINTLN( "Location {} {} already has a ship hull present", desired_pos.realm()->name(),
                    hullpos );
#endif
      return false;
    }

    if ( !desired_pos.realm()->navigable( hullpos, Plib::systemstate.tile[ele->objtype].height ) )
      return false;
  }

  return true;
}

// ugh, we've moved the ship already - but we're comparing using the character's coords
// which used to be on the ship.
// let's hope it's always illegal to stand on the edges. !!
bool UBoat::on_ship( const BoatContext& bc, const UObject* obj )
{
  if ( Core::IsItem( obj->serial ) )
  {
    const Item* item = static_cast<const Item*>( obj );
    if ( item->container != nullptr )
      return false;
  }
  Core::Vec2d rxy = obj->pos2d() - bc.oldpos.xy();

  return bc.mdef.body_contains( rxy );
}

void UBoat::move_travellers( const BoatContext& oldlocation )
{
  bool any_orphans = false;

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    // consider: occasional sweeps of all boats to reap orphans
    if ( obj->orphan() || !on_ship( oldlocation, obj ) )
    {
      any_orphans = true;
      travellerRef.clear();
      continue;
    }

    obj->set_dirty();
    // keeps relative distance from boat mast
    auto delta = obj->pos3d() - oldlocation.oldpos.xyz();
    auto newtravellerpos = pos() + delta;
    if ( obj->ismobile() )
    {
      auto* chr = static_cast<Mobile::Character*>( obj );
      move_boat_mobile( chr, newtravellerpos );
    }
    else
    {
      auto* item = static_cast<Items::Item*>( obj );
      move_boat_item( item, newtravellerpos );

      if ( Core::settingsManager.ssopt.refresh_decay_after_boat_moves )
        item->restart_decay_timer();
    }
  }

  if ( any_orphans )
    remove_orphans();
}

void UBoat::move_boat_item( Items::Item* item, const Core::Pos4d& newpos )
{
  item->set_dirty();
  const Core::Pos4d oldpos = item->pos();
  item->setposition( newpos );
  MoveItemWorldPosition( oldpos, item );
  // TODO POS should be removed
  if ( oldpos.realm() != newpos.realm() && item->isa( Core::UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    auto* cont = static_cast<Core::UContainer*>( item );
    cont->for_each_item( Core::setrealm, (void*)realm() );
  }

  Core::WorldIterator<Core::OnlinePlayerFilter>::InMaxVisualRange(
      item,
      [&]( Mobile::Character* zonechr )
      {
        if ( !zonechr->in_visual_range( item ) )
          return;
        Network::Client* client = zonechr->client;

        if ( !( client->ClientType & Network::CLIENTTYPE_7090 ) )
          send_item( client, item );
        else if ( !client->chr->in_visual_range(
                      item, oldpos ) )  // multis are visible before a client accepts items, we need
                                        // to resend them
          send_item( client, item );
      } );

  Core::WorldIterator<Core::OnlinePlayerFilter>::InMaxVisualRange(
      oldpos,
      [&]( Mobile::Character* zonechr )
      {
        if ( !zonechr->in_visual_range( item, oldpos ) )
          return;
        if ( !zonechr->in_visual_range(
                 item ) )  // not in range.  If old loc was in range, send a delete.
          send_remove_object( zonechr->client, item );
      } );
}

void UBoat::move_boat_mobile( Mobile::Character* chr, const Core::Pos4d& newpos )
{
  const auto oldpos = chr->pos();
  // TODO from core.cpp more refactoring... can move_character_to() be used here?
  if ( oldpos.realm() != newpos.realm() )
  {
    oldpos.realm()->notify_left( *chr );
    send_remove_character_to_nearby( chr );
    if ( chr->client != nullptr )
      Core::remove_objects_inrange( chr->client );
  }

  chr->setposition( newpos );
  chr->lastpos = oldpos;
  if ( oldpos.realm() != newpos.realm() )
    chr->realm_changed();

  MoveCharacterWorldPosition( oldpos, chr );
  chr->position_changed();
  if ( chr->has_active_client() )
  {
    if ( oldpos.realm() != chr->realm() )
    {
      Core::send_new_subserver( chr->client );
      Core::send_owncreate( chr->client, chr );
    }

    if ( chr->client->ClientType & Network::CLIENTTYPE_7090 )
    {
      if ( chr->poisoned() )  // if poisoned send 0x17 for newer clients
        send_poisonhealthbar( chr->client, chr );

      if ( chr->invul() )  // if invul send 0x17 for newer clients
        send_invulhealthbar( chr->client, chr );
    }
    else
    {
      Core::send_goxyz( chr->client, chr );
      // lastpos are set above so these two calls will work right.
      // FIXME these are also called, in this order, in MOVEMENT.CPP.
      // should be consolidated.
      Core::send_objects_newly_inrange_on_boat( chr->client, this->serial );
    }
  }
  chr->move_reason = Mobile::Character::MULTIMOVE;
  // multis are visible before a client accepts objects, we need to resend them
  Core::WorldIterator<Core::OnlinePlayerFilter>::InMaxVisualRange(
      chr,
      [&]( Mobile::Character* zonechr )
      {
        if ( !( zonechr->client->ClientType & Network::CLIENTTYPE_7090 ) )
          return;
        if ( !zonechr->is_visible_to_me( chr, /*check_range*/ true ) )
          return;
        if ( !zonechr->in_visual_range( chr, oldpos ) )
          send_owncreate( zonechr->client, chr );
      } );
}

Core::Pos4d UBoat::turn_coords( const Core::Pos4d& oldpos, RELATIVE_DIR dir ) const
{
  Core::Vec3d delta = oldpos.xyz() - pos3d();
  switch ( dir )
  {
  case LEFT:
    delta = Core::Vec3d( delta.y(), -delta.x(), delta.z() );
    break;
  case AROUND:
    delta.x( -delta.x() ).y( -delta.y() );
    break;
  case RIGHT:
    delta = Core::Vec3d( -delta.y(), delta.x(), delta.z() );
    break;
  case NO_TURN:
    return oldpos;
  }
  return pos() + delta;
}

u8 UBoat::turn_facing( u8 oldfacing, RELATIVE_DIR dir ) const
{
  return ( ( dir * 2 ) + oldfacing ) & 7;
}

void UBoat::turn_travellers( RELATIVE_DIR dir, const BoatContext& oldlocation )
{
  bool any_orphans = false;

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    // consider: occasional sweeps of all boats to reap orphans
    if ( obj->orphan() || !on_ship( oldlocation, obj ) )
    {
      any_orphans = true;
      travellerRef.clear();
      continue;
    }

    obj->set_dirty();
    auto newpos = turn_coords( obj->pos(), dir );
    if ( obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      chr->setfacing( turn_facing( chr->facing, dir ) );
      move_boat_mobile( chr, newpos );
    }
    else
    {
      Items::Item* item = static_cast<Items::Item*>( obj );

      move_boat_item( item, newpos );
      if ( Core::settingsManager.ssopt.refresh_decay_after_boat_moves )
        item->restart_decay_timer();
    }
  }

  if ( any_orphans )
    remove_orphans();
}

void UBoat::remove_orphans()
{
  bool any_orphan_travellers;
  do
  {
    any_orphan_travellers = false;

    for ( Travellers::iterator itr = travellers_.begin(), end = travellers_.end(); itr != end;
          ++itr )
    {
      UObject* obj = ( *itr ).get();
      if ( obj == nullptr )
      {
        set_dirty();
        travellers_.erase( itr );
        any_orphan_travellers = true;
        break;
      }
    }
  } while ( any_orphan_travellers );
}

void UBoat::cleanup_deck()
{
  BoatContext bc( *this );

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    if ( obj->orphan() || !on_ship( bc, obj ) )
    {
      set_dirty();
      travellerRef.clear();
    }
  }
  remove_orphans();
}

bool UBoat::has_offline_mobiles() const
{
  BoatContext bc( *this );

  for ( const auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    if ( !obj->orphan() && on_ship( bc, obj ) && obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );

      if ( !chr->logged_in() )
      {
        return true;
      }
    }
  }
  return false;
}

void UBoat::move_offline_mobiles( const Core::Pos4d& newpos )
{
  BoatContext bc( *this );

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    if ( !obj->orphan() && on_ship( bc, obj ) && obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );

      if ( !chr->logged_in() )
      {
        chr->set_dirty();
        chr->setposition( newpos );
        chr->realm_changed();  // not sure if neccessary...
        travellerRef.clear();
      }
    }
  }
  remove_orphans();
}

void UBoat::on_color_changed()
{
  send_display_boat_to_inrange( {} );
}

bool UBoat::deck_empty() const
{
  return travellers_.empty();
}

bool UBoat::hold_empty() const
{
  Items::Item* it = this->hold;
  if ( it == nullptr )
  {
    return true;
  }
  Core::UContainer* cont = static_cast<Core::UContainer*>( it );
  return ( cont->count() == 0 );
}

void UBoat::do_tellmoves()
{
  // we only do tellmove here because tellmove also checks attacks.
  // this way, we move everyone, then check for attacks.

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    if ( !obj || !obj->ismobile() )
      continue;
    auto* chr = static_cast<Mobile::Character*>( obj );
    if ( chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
    {
      chr->tellmove();
      continue;
    }
    if ( !chr->has_active_client() )
      continue;
    chr->tellmove();
    auto* client = chr->client;
    // with smooth movement the position of travellers is known when the displayboat pkt is send,
    // new objects have to be send afterwards
    if ( client->ClientType & Network::CLIENTTYPE_7090 )
      Core::send_objects_newly_inrange_on_boat( client, serial );
  }
}

bool UBoat::move_to( const Core::Pos4d& newpos, int flags )
{
  BoatMoveGuard registerguard( this );
  if ( ( flags & Core::MOVEITEM_FORCELOCATION ) || navigable( multidef(), newpos ) )
  {
    BoatContext bc( *this );

    set_dirty();

    setposition( newpos );
    move_multi_in_world( this, bc.oldpos );

    move_travellers( bc );
    move_components();
    send_display_boat_to_inrange( bc.oldpos );
    do_tellmoves();
    unpause_paused();

    return true;
  }
  return false;
}

bool UBoat::move( Core::UFACING dir, u8 speed, bool relative )
{
  BoatMoveGuard registerguard( this );

  Core::UFACING move_dir;

  if ( relative == false )
    move_dir = dir;
  else
    move_dir = static_cast<Core::UFACING>( ( dir + boat_facing() ) & 7 );

  auto newpos = pos().move( move_dir );

  if ( !navigable( multidef(), newpos ) )
    return false;

  BoatContext bc( *this );

  set_dirty();

  setposition( newpos );
  move_multi_in_world( this, bc.oldpos );
  move_travellers( bc );
  move_components();

  Core::WorldIterator<Core::OnlinePlayerFilter>::InMaxVisualRange(
      this,
      [&]( Mobile::Character* zonechr )
      {
        Network::Client* client = zonechr->client;
        if ( !zonechr->in_visual_range( this ) )
          return;
        if ( client->ClientType & Network::CLIENTTYPE_7090 )
        {
          if ( zonechr->in_visual_range( this, bc.oldpos ) )
            send_smooth_move( client, move_dir, speed, relative );
          else
            send_boat_newly_inrange( client );  // send HSA packet only for newly inrange
        }
        else
        {
          if ( client->ClientType & Network::CLIENTTYPE_7000 )
            send_boat( client );  // Send
          else
            send_boat_old( client );
        }
      } );

  Core::WorldIterator<Core::OnlinePlayerFilter>::InMaxVisualRange(
      bc.oldpos,
      [&]( Mobile::Character* zonechr )
      {
        if ( zonechr->in_visual_range( this, bc.oldpos ) &&
             !zonechr->in_visual_range( this ) )  // send remove to chrs only seeing the old loc
          send_remove_boat( zonechr->client );
      } );

  do_tellmoves();
  unpause_paused();
  return true;
}

inline unsigned short UBoat::multiid_ifturn( RELATIVE_DIR dir )
{
  return ( multiid_ & ~3u ) | ( ( multiid_ + dir ) & 3 );
}

const MultiDef& UBoat::multi_ifturn( RELATIVE_DIR dir )
{
  unsigned short multiid_dir = multiid_ifturn( dir );
  passert( MultiDefByMultiIDExists( multiid_dir ) );
  return *MultiDefByMultiID( multiid_dir );
}

Core::UFACING UBoat::boat_facing() const
{
  return static_cast<Core::UFACING>( ( multiid_ & 3 ) * 2 );
}

const BoatShape& UBoat::boatshape() const
{
  passert( Core::gamestate.boatshapes.count( multiid_ ) != 0 );
  return *Core::gamestate.boatshapes[multiid_];
}


void UBoat::transform_components( const BoatShape& old_boatshape )
{
  const BoatShape& bshape = boatshape();
  auto end = Components.end();
  auto end2 = bshape.Componentshapes.end();
  auto old_end = old_boatshape.Componentshapes.end();

  auto itr = Components.begin();
  auto itr2 = bshape.Componentshapes.begin();
  auto old_itr = old_boatshape.Componentshapes.begin();

  for ( ; itr != end && itr2 != end2 && old_itr != old_end; ++itr, ++itr2, ++old_itr )
  {
    Items::Item* item = itr->get();
    if ( item != nullptr )
    {
      if ( item->orphan() )
        continue;

      // This should be rare enough for a simple log to be the solution. We don't want POL to
      // crash in MoveItemWorldPosition() because the item was not in the world to start with, so
      // we skip it.
      if ( item->container != nullptr || item->has_gotten_by() )
      {
        u32 containerSerial = ( item->container != nullptr ) ? item->container->serial : 0;
        POLLOG_ERRORLN(
            "Boat component is gotten or in a container and couldn't be moved together with the "
            "boat: serial {:#x}\n, graphic: {:#x}, container: {:#x}.",
            item->serial, item->graphic, containerSerial );
        continue;
      }

      if ( item->objtype_ == Core::settingsManager.extobj.port_plank &&
           item->graphic == old_itr->altgraphic )
        item->graphic = itr2->altgraphic;
      else if ( item->objtype_ == Core::settingsManager.extobj.starboard_plank &&
                item->graphic == old_itr->altgraphic )
        item->graphic = itr2->altgraphic;
      else
        item->graphic = itr2->graphic;

      move_boat_item( item, pos() + itr2->delta );
    }
  }
}

void UBoat::move_components()
{
  const BoatShape& bshape = boatshape();
  auto itr = Components.begin();
  auto end = Components.end();
  auto itr2 = bshape.Componentshapes.begin();
  auto end2 = bshape.Componentshapes.end();
  for ( ; itr != end && itr2 != end2; ++itr, ++itr2 )
  {
    Items::Item* item = itr->get();
    if ( item != nullptr )
    {
      if ( item->orphan() )
      {
        continue;
      }

      // This should be rare enough for a simple log to be the solution. We don't want POL to
      // crash in MoveItemWorldPosition() because the item was not in the world to start with, so
      // we skip it.
      if ( item->container != nullptr || item->has_gotten_by() )
      {
        u32 containerSerial = ( item->container != nullptr ) ? item->container->serial : 0;
        POLLOG_INFOLN(
            "Boat component is gotten or in a container and couldn't be moved together with the "
            "boat: serial {:#x}\n"
            ", graphic: {:#x}, container: {:#x}.",
            item->serial, item->graphic, containerSerial );
        continue;
      }
      move_boat_item( item, pos() + itr2->delta );
    }
  }
}

bool UBoat::turn( RELATIVE_DIR dir )
{
  BoatMoveGuard registerguard( this );

  const MultiDef& newmd = multi_ifturn( dir );

  if ( !navigable( newmd, pos() ) )
    return false;
  BoatContext bc( *this );
  const BoatShape& old_boatshape = boatshape();

  set_dirty();
  multiid_ = multiid_ifturn( dir );

  turn_travellers( dir, bc );
  transform_components( old_boatshape );
  send_display_boat_to_inrange( {} );
  do_tellmoves();
  unpause_paused();
  facing = turn_facing( facing, dir );
  return true;
}

void UBoat::register_object( UObject* obj )
{
  if ( find( travellers_.begin(), travellers_.end(), obj ) == travellers_.end() )
  {
    set_dirty();
    travellers_.emplace_back( obj );
  }
}

void UBoat::unregister_object( UObject* obj )
{
  Travellers::iterator this_traveller = find( travellers_.begin(), travellers_.end(), obj );

  if ( this_traveller != travellers_.end() )
  {
    set_dirty();
    travellers_.erase( this_traveller );
  }
}

void UBoat::rescan_components()
{
  UPlank* plank;

  if ( portplank != nullptr && !portplank->orphan() )
  {
    plank = static_cast<UPlank*>( portplank );
    plank->setboat( this );
  }

  if ( starboardplank != nullptr && !starboardplank->orphan() )
  {
    plank = static_cast<UPlank*>( starboardplank );
    plank->setboat( this );
  }
}

void UBoat::reread_components()
{
  for ( auto& component : Components )
  {
    if ( component == nullptr )
      continue;
    // check boat members here
    if ( component->objtype_ == Core::settingsManager.extobj.tillerman && tillerman == nullptr )
      tillerman = component.get();
    if ( component->objtype_ == Core::settingsManager.extobj.port_plank && portplank == nullptr )
      portplank = component.get();
    if ( component->objtype_ == Core::settingsManager.extobj.starboard_plank &&
         starboardplank == nullptr )
      starboardplank = component.get();
    if ( component->objtype_ == Core::settingsManager.extobj.hold && hold == nullptr )
      hold = component.get();
  }
}

/// POL098 and earlier were using graphic to store MultiID, this should not be lost
/// to avoid screwing up boats during conversion
void UBoat::fixInvalidGraphic()
{
  if ( !Core::settingsManager.polvar.DataWrittenBy99OrLater )
  {
    passert_always_r( graphic >= 0x4000, "Unexpected boat graphic < 0x4000 in POL < 099 data" );
    multiid_ = graphic - 0x4000;
  }
  base::fixInvalidGraphic();
}

void UBoat::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );

  BoatContext bc( *this );
  u32 tmp_serial;
  while ( elem.remove_prop( "Traveller", &tmp_serial ) )
  {
    if ( Core::IsItem( tmp_serial ) )
    {
      Items::Item* item = Core::find_toplevel_item( tmp_serial );
      if ( item != nullptr )
      {
        if ( BoatShape::objtype_is_component( item->objtype_ ) )
        {
          Components.emplace_back( item );
        }
        else if ( on_ship( bc, item ) )
        {
          travellers_.emplace_back( item );
        }
      }
    }
    else
    {
      Mobile::Character* chr = Core::system_find_mobile( tmp_serial );

      if ( chr != nullptr )
      {
        if ( on_ship( bc, chr ) )
          travellers_.emplace_back( chr );
      }
    }
  }
  while ( elem.remove_prop( "Component", &tmp_serial ) )
  {
    Items::Item* item = Core::system_find_item( tmp_serial );
    if ( item != nullptr )
    {
      if ( BoatShape::objtype_is_component( item->objtype_ ) )
      {
        Components.emplace_back( item );
      }
    }
  }
  reread_components();
  rescan_components();

  regself();  // do this after our x,y are known.
  // consider throwing if starting position isn't passable.

  auto control_script = itemdesc().control_script.empty() ? Core::ScriptDef( "misc/boat", nullptr )
                                                          : itemdesc().control_script;

  auto prog = Core::find_script2( control_script );

  if ( prog.get() == nullptr )
  {
    POLLOG_ERRORLN( "Could not start script {}, boat: serial {:#x}", control_script.c_str(),
                    this->serial );
    return;
  }

  Module::UOExecutorModule* script = Core::start_script( prog, make_boatref( this ) );

  if ( script == nullptr )
  {
    POLLOG_ERRORLN( "Could not start script {}, boat: serial {:#x}", control_script.c_str(),
                    this->serial );
  }
  else
  {
    this->process( script );
    this->process()->attached_item_.set( this );
  }
}

void UBoat::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );

  sw.add( "MultiID", multiid_ );

  BoatContext bc( *this );

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();
    if ( !obj->orphan() && on_ship( bc, obj ) )
    {
      sw.add( "Traveller", Clib::hexintv( obj->serial ) );
    }
  }
  for ( auto& component : Components )
  {
    if ( component != nullptr && !component->orphan() )
    {
      sw.add( "Component", Clib::hexintv( component->serial ) );
    }
  }
}

Bscript::BObjectImp* UBoat::scripted_create( const Items::ItemDesc& descriptor,
                                             const Core::Pos4d& pos, int flags )
{
  unsigned short multiid = descriptor.multiid;
  unsigned short multiid_offset =
      static_cast<unsigned short>( ( flags & CRMULTI_FACING_MASK ) >> CRMULTI_FACING_SHIFT );
  unsigned char facing = static_cast<unsigned char>( multiid_offset * 2 );
  multiid += multiid_offset;

  const MultiDef* md = MultiDefByMultiID( multiid );
  if ( md == nullptr )
  {
    return new Bscript::BError(
        "Multi definition not found for Boat, objtype=" + Clib::hexint( descriptor.objtype ) +
        ", multiid=" + Clib::hexint( multiid ) );
  }
  if ( !Core::gamestate.boatshapes.count( descriptor.multiid ) )
  {
    return new Bscript::BError(
        "No boatshape for Boat in boats.cfg, objtype=" + Clib::hexint( descriptor.objtype ) +
        ", multiid=" + Clib::hexint( multiid ) );
  }

  if ( !navigable( *md, pos ) )
  {
    return new Bscript::BError( "Position indicated is impassable" );
  }

  UBoat* boat = new UBoat( descriptor );
  boat->multiid_ = multiid;
  boat->serial = Core::GetNewItemSerialNumber();
  boat->serial_ext = ctBEu32( boat->serial );
  boat->setposition( pos );
  boat->facing = facing;
  add_multi_to_world( boat );
  boat->send_display_boat_to_inrange( {} );
  boat->create_components();
  boat->rescan_components();
  unpause_paused();
  boat->regself();

  ////hash
  Core::objStorageManager.objecthash.Insert( boat );
  ////

  Bscript::BObjectImp* boatref = make_boatref( boat );

  auto control_script = descriptor.control_script.empty() ? Core::ScriptDef( "misc/boat", nullptr )
                                                          : descriptor.control_script;

  auto prog = Core::find_script2( control_script );

  if ( prog.get() == nullptr )
  {
    POLLOG_ERRORLN( "Could not start script {}, boat: serial {:#x}", control_script.c_str(),
                    boat->serial );

    return boatref;
  }

  Module::UOExecutorModule* script = Core::start_script( prog, boatref );

  if ( script == nullptr )
  {
    POLLOG_ERRORLN( "Could not start script {}, boat: serial {:#x}", control_script.c_str(),
                    boat->serial );
  }
  else
  {
    boat->process( script );
    boat->process()->attached_item_.set( boat );
  }

  return boatref;
}

void UBoat::create_components()
{
  const BoatShape& bshape = boatshape();
  for ( std::vector<BoatShape::ComponentShape>::const_iterator itr = bshape.Componentshapes.begin(),
                                                               end = bshape.Componentshapes.end();
        itr != end; ++itr )
  {
    Items::Item* component;
    try
    {
      component = Items::Item::create( itr->objtype );
    }
    catch ( ... )
    {
      continue;
    }
    if ( component == nullptr )
      continue;
    // check boat members here
    if ( component->objtype_ == Core::settingsManager.extobj.tillerman && tillerman == nullptr )
      tillerman = component;
    if ( component->objtype_ == Core::settingsManager.extobj.port_plank && portplank == nullptr )
      portplank = component;
    if ( component->objtype_ == Core::settingsManager.extobj.starboard_plank &&
         starboardplank == nullptr )
      starboardplank = component;
    if ( component->objtype_ == Core::settingsManager.extobj.hold && hold == nullptr )
      hold = component;

    component->graphic = itr->graphic;
    // component itemdesc entries generally have graphic=1, so they don't get their height set.
    component->height = Plib::tileheight( component->graphic );
    component->setposition( pos() + itr->delta );
    component->disable_decay();
    component->movable( false );
    add_item_to_world( component );
    update_item_to_inrange( component );
    Components.emplace_back( component );
  }
}

Bscript::BObjectImp* UBoat::items_list() const
{
  BoatContext bc( *this );
  Bscript::ObjArray* arr = new Bscript::ObjArray;

  for ( const auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();
    if ( !obj->orphan() && on_ship( bc, obj ) && Core::IsItem( obj->serial ) )
    {
      Item* item = static_cast<Item*>( obj );
      arr->addElement( make_itemref( item ) );
    }
  }
  return arr;
}

Bscript::BObjectImp* UBoat::mobiles_list() const
{
  BoatContext bc( *this );
  Bscript::ObjArray* arr = new Bscript::ObjArray;
  for ( const auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();
    if ( !obj->orphan() && on_ship( bc, obj ) && obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( chr->logged_in() )
        arr->addElement( make_mobileref( chr ) );
    }
  }
  return arr;
}

Bscript::BObjectImp* UBoat::component_list( unsigned char type ) const
{
  Bscript::ObjArray* arr = new Bscript::ObjArray;
  for ( const auto& component : Components )
  {
    if ( component != nullptr && !component->orphan() )
    {
      if ( type == COMPONENT_ALL )
      {
        arr->addElement( component->make_ref() );
      }
      else
      {
        if ( component->objtype_ == get_component_objtype( type ) )
          arr->addElement( component->make_ref() );
      }
    }
  }
  return arr;
}

void UBoat::destroy_components()
{
  for ( auto& component : Components )
  {
    if ( component != nullptr && !component->orphan() )
    {
      Core::destroy_item( component.get() );
    }
  }
  Components.clear();
}

size_t UBoat::estimatedSize() const
{
  size_t size = base::estimatedSize() + sizeof( Items::Item* ) /*tillerman*/
                + sizeof( Items::Item* )                       /*portplank*/
                + sizeof( Items::Item* )                       /*starboardplank*/
                + sizeof( Items::Item* )                       /*hold*/
                // no estimateSize here element is in objhash
                + Clib::memsize( travellers_ ) + Clib::memsize( Components );
  return size;
}

bool UBoat::get_method_hook( const char* methodname, Bscript::Executor* ex,
                             Core::ExportScript** hook, unsigned int* PC ) const
{
  if ( Core::gamestate.system_hooks.get_method_hook(
           Core::gamestate.system_hooks.boat_method_script.get(), methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}

Bscript::BObjectImp* destroy_boat( UBoat* boat )
{
  boat->cleanup_deck();

  if ( !boat->hold_empty() )
    return new Bscript::BError( "There is cargo in the ship's hold" );
  if ( boat->has_offline_mobiles() )
    return new Bscript::BError( "There are logged-out characters on the deck" );
  if ( !boat->deck_empty() )
    return new Bscript::BError( "The deck is not empty" );

  boat->destroy_components();
  boat->unregself();

  Core::WorldIterator<Core::OnlinePlayerFilter>::InMaxVisualRange(
      boat,
      [&]( Mobile::Character* zonechr )
      {
        if ( zonechr->in_visual_range( boat ) )
          Core::send_remove_object( zonechr->client, boat );
      } );
  remove_multi_from_world( boat );
  boat->destroy();
  return new Bscript::BLong( 1 );
}

Mobile::Character* UBoat::pilot() const
{
  if ( mountpiece != nullptr && !mountpiece->orphan() )
  {
    return mountpiece->GetCharacterOwner();
  }
  return nullptr;
}

Bscript::BObjectImp* UBoat::set_pilot( Mobile::Character* chr )
{
  if ( chr == nullptr )
  {
    clear_pilot();
    return new Bscript::BLong( 1 );
  }

  if ( mountpiece != nullptr && !mountpiece->orphan() )
  {
    return new Bscript::BError( "The boat is already being piloted." );
  }

  if ( !has_process() )
  {
    return new Bscript::BError( "The boat does not have a running process." );
  }

  if ( !chr->client )
  {
    return new Bscript::BError( "That character is not connected." );
  }

  if ( !( chr->client->ClientType & Network::CLIENTTYPE_7090 ) )
  {
    return new Bscript::BError(
        "The client for that character does not support High Seas Adventure." );
  }

  BoatContext bc( *this );
  bool pilot_on_ship = false;
  for ( const auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();
    if ( !obj->orphan() && on_ship( bc, obj ) && obj == chr )
    {
      pilot_on_ship = true;
      break;
    }
  }

  if ( !pilot_on_ship )
  {
    return new Bscript::BError( "The boat does not have that character on it." );
  }

  Items::Item* item = Items::Item::create( Core::settingsManager.extobj.boatmount );
  if ( !chr->equippable( item ) )
  {
    item->destroy();
    return new Bscript::BError( "The boat mount piece is not equippable by that character." );
  }
  chr->equip( item );
  send_wornitem_to_inrange( chr, item );
  mountpiece = Core::ItemRef( item );

  // Mark the item as 'in-use' to prevent moving by client or scripts.
  item->inuse( true );

  return new Bscript::BLong( 1 );
}

void UBoat::clear_pilot()
{
  if ( mountpiece != nullptr )
  {
    if ( !mountpiece->orphan() )
    {
      destroy_item( mountpiece.get() );
    }
    mountpiece.clear();
  }
}
}  // namespace Pol::Multi
