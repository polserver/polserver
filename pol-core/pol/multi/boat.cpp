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
#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/streamsaver.h"
#include "../../plib/systemstate.h"
#include "../../plib/tiles.h"
#include "../../plib/uconst.h"
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
#include "../network/client.h"
#include "../network/packethelper.h"
#include "../network/packets.h"
#include "../network/pktdef.h"
#include "../polvar.h"
#include "../realms/realm.h"
#include "../scrsched.h"
#include "../syshookscript.h"
#include "../ufunc.h"
#include "../uobject.h"
#include "../uworld.h"
#include "boatcomp.h"
#include "multi.h"
#include "multidef.h"


namespace Pol
{
namespace Multi
{
//#define DEBUG_BOATS

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
      unsigned short xd, yd;
      if ( is >> xd >> yd )
      {
        xdelta = xd;
        ydelta = yd;
        zdelta = 0;
        signed short zd;
        if ( is >> zd )
          zdelta = zd;
        return;
      }
    }
  }

  ERROR_PRINT << "Boat component definition '" << str << "' is poorly formed.\n";
  throw std::runtime_error( "Poorly formed boat.cfg component definition" );
}

BoatShape::ComponentShape::ComponentShape( const std::string& str, const std::string& altstr,
                                           unsigned char type )
{
  altgraphic = 0;
  bool ok = false;
  objtype = get_component_objtype( type );
  ISTRINGSTREAM is( str );
  std::string tmp;
  if ( is >> tmp )
  {
    graphic = static_cast<unsigned short>( strtoul( tmp.c_str(), nullptr, 0 ) );
    if ( graphic )
    {
      unsigned short xd, yd;
      if ( is >> xd >> yd )
      {
        xdelta = xd;
        ydelta = yd;
        zdelta = 0;
        signed short zd;
        if ( is >> zd )
          zdelta = zd;
        ok = true;
      }
    }
  }

  ISTRINGSTREAM altis( altstr );
  std::string alttmp;
  if ( ok && altis >> alttmp )
  {
    altgraphic = static_cast<unsigned short>( strtoul( alttmp.c_str(), nullptr, 0 ) );
    return;
  }
  else
    ok = false;

  if ( !ok )
  {
    ERROR_PRINT << "Boat component definition '" << str << "' is poorly formed.\n";
    throw std::runtime_error( "Poorly formed boat.cfg component definition" );
  }
}


BoatShape::BoatShape() {}
BoatShape::BoatShape( Clib::ConfigElem& elem )
{
  std::string tmp_str;

  while ( elem.remove_prop( "Tillerman", &tmp_str ) )
    Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_TILLERMAN ) );
  while ( elem.remove_prop( "PortGangplankRetracted", &tmp_str ) )
    Componentshapes.push_back( ComponentShape(
        tmp_str, elem.remove_string( "PortGangplankExtended" ), COMPONENT_PORT_PLANK ) );
  while ( elem.remove_prop( "StarboardGangplankRetracted", &tmp_str ) )
    Componentshapes.push_back( ComponentShape(
        tmp_str, elem.remove_string( "StarboardGangplankExtended" ), COMPONENT_STARBOARD_PLANK ) );
  while ( elem.remove_prop( "Hold", &tmp_str ) )
    Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_HOLD ) );
  while ( elem.remove_prop( "Rope", &tmp_str ) )
    Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_ROPE ) );
  while ( elem.remove_prop( "Wheel", &tmp_str ) )
    Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_WHEEL ) );
  while ( elem.remove_prop( "Hull", &tmp_str ) )
    Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_HULL ) );
  while ( elem.remove_prop( "Tiller", &tmp_str ) )
    Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_TILLER ) );
  while ( elem.remove_prop( "Rudder", &tmp_str ) )
    Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_RUDDER ) );
  while ( elem.remove_prop( "Sails", &tmp_str ) )
    Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_SAILS ) );
  while ( elem.remove_prop( "Storage", &tmp_str ) )
    Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_STORAGE ) );
  while ( elem.remove_prop( "Weaponslot", &tmp_str ) )
    Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_WEAPONSLOT ) );
}

bool BoatShape::objtype_is_component( unsigned int objtype )
{
  if ( objtype == Core::settingsManager.extobj.tillerman )
    return true;
  else if ( objtype == Core::settingsManager.extobj.port_plank )
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
  return 3 * sizeof( ComponentShape* ) + Componentshapes.capacity() * sizeof( ComponentShape );
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

void read_boat_cfg( void )
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
      ERROR_PRINT << "Error occurred reading definition for boat 0x" << fmt::hexu( multiid )
                  << "\n";
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

// TODO: Updating the objects and components before calling this function would help!
void UBoat::send_smooth_move( Network::Client* client, Core::UFACING bowrelative_dir, u8 speed,
                              const Core::Pos2d& newpos )
{
  Network::PktHelper::PacketOut<Network::PktOut_F6> msg;

  const Core::Vec2d displacement = newpos - pos().xy();

  msg->offset += 2;  // Length
  msg->Write<u32>( serial_ext );
  msg->Write<u8>( speed );

  msg->Write<u8>( bowrelative_dir );
  msg->Write<u8>( boat_facing() );

  msg->WriteFlipped<u16>( newpos.x() );
  msg->WriteFlipped<u16>( newpos.y() );
  // TODO: Isn't 0x10000+z just sign-extending negative z to 16 bits (should be the same as
  // WriteFlipped<s16>)?
  msg->WriteFlipped<u16>( ( this->z() < 0 ) ? static_cast<u16>( 0x10000 + this->z() )
                                            : static_cast<u16>( this->z() ) );

  u16 object_count = static_cast<u16>( travellers_.size() + Components.size() );

  msg->WriteFlipped<u16>( object_count );

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    if ( !obj->orphan() )
    {
      Core::Pos2d obj_newpos = obj->pos().xy() + displacement;
      msg->Write<u32>( obj->serial_ext );
      msg->WriteFlipped<u16>( static_cast<u16>( obj_newpos.x() ) );
      msg->WriteFlipped<u16>( static_cast<u16>( obj_newpos.y() ) );
      msg->WriteFlipped<u16>( ( obj->z() < 0 ) ? static_cast<u16>( 0x10000 + obj->z() )
                                               : static_cast<u16>( obj->z() ) );
    }
  }

  for ( auto& component : Components )
  {
    if ( component != nullptr && !component->orphan() )
    {
      Core::Pos2d component_newpos = component->pos().xy() + displacement;
      msg->Write<u32>( component->serial_ext );
      msg->WriteFlipped<u16>( static_cast<u16>( component_newpos.x() ) );
      msg->WriteFlipped<u16>( static_cast<u16>( component_newpos.y() ) );
      msg->WriteFlipped<u16>( static_cast<u16>(
          ( component->z() < 0 ) ? ( 0x10000 + component->z() ) : ( component->z() ) ) );
    }
  }

  u16 len = msg->offset;

  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  msg.Send( client, len );
}

// TODO: This is building the whole smooth move packet again and again. Surely it can be optimized?
void UBoat::send_smooth_move_to_inrange( Core::UFACING bowrelative_dir, u8 speed,
                                         const Core::Pos2d& newpos )
{
  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      newpos, this->realm(), RANGE_VISUAL_LARGE_BUILDINGS, [&]( Mobile::Character* zonechr ) {
        Network::Client* client = zonechr->client;

        if ( inrange( client->chr, this ) &&
             client->ClientType & Network::CLIENTTYPE_7090 )  // send this only to those who see the
                                                              // old location aswell
          send_smooth_move( client, bowrelative_dir, speed, newpos );
      } );
}

void UBoat::send_display_boat( Network::Client* client )
{
  Network::PktHelper::PacketOut<Network::PktOut_F7> msg;

  msg->offset += 2;  // Length

  u16 inner_packet_count =
      static_cast<u16>( travellers_.size() + Components.size() + 1 );  // Add 1 for the boat aswell

  msg->WriteFlipped<u16>( inner_packet_count );

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

  u8 flags = 0;

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    if ( !obj->orphan() )
    {
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
        flags = 0;

        msg->WriteFlipped<u16>( 0x1u );  // Amount
        msg->WriteFlipped<u16>( 0x1u );  // Amount
      }
      else
      {
        Items::Item* item = static_cast<Items::Item*>( obj );

        if ( item->invisible() && !client->chr->can_seeinvisitems() )
        {
          send_remove_object( client, item );
          continue;
        }

        if ( client->chr->can_move( item ) )
          flags |= ITEM_FLAG_FORCE_MOVABLE;

        msg->WriteFlipped<u16>( item->get_senditem_amount() );  // Amount
        msg->WriteFlipped<u16>( item->get_senditem_amount() );  // Amount
      }

      msg->WriteFlipped<u16>( obj->x() );
      msg->WriteFlipped<u16>( obj->y() );
      msg->Write<s8>( obj->z() );
      msg->Write<u8>( obj->facing );
      msg->WriteFlipped<u16>( obj->color );

      msg->Write<u8>( flags );  // FLAGS
      msg->offset += 2;  // HSA access flags, TODO find out what these are for and implement it
    }
  }

  for ( auto& component : Components )
  {
    if ( component != nullptr && !component->orphan() )
    {
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
    }
  }

  u16 len = msg->offset;

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
  msg->WriteFlipped<u16>( this->x() );
  msg->WriteFlipped<u16>( this->y() );
  msg->Write<s8>( this->z() );
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

void UBoat::send_display_boat_to_inrange( const Core::Pos4d& oldpos )
{
  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      this->pos(), RANGE_VISUAL_LARGE_BUILDINGS, [&]( Mobile::Character* zonechr ) {
        Network::Client* client = zonechr->client;

        if ( client->ClientType & Network::CLIENTTYPE_7090 )
          send_display_boat( client );
        else if ( client->ClientType & Network::CLIENTTYPE_7000 )
          send_boat( client );
        else
          send_boat_old( client );
      } );

  if ( oldpos != this->pos() )
  {
    Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
        oldpos, RANGE_VISUAL_LARGE_BUILDINGS, [&]( Mobile::Character* zonechr ) {
          Network::Client* client = zonechr->client;

          if ( !inrange( client->chr, this ) )  // send remove to chrs only seeing the old loc
            send_remove_boat( client );
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


UBoat::UBoat( const Items::ItemDesc& descriptor ) : UMulti( descriptor )
{
  passert( Core::gamestate.boatshapes.count( multiid ) != 0 );
  tillerman = nullptr;
  hold = nullptr;
  portplank = nullptr;
  starboardplank = nullptr;
}

UBoat* UBoat::as_boat()
{
  return this;
}

/* boats are a bit strange - they can move from side to side, forwards and
   backwards, without turning.
   */
void UBoat::regself()
{
  const MultiDef& md = multidef();
  for ( auto itr = md.hull.begin(), end = md.hull.end(); itr != end; ++itr )
  {
    Core::Pos3d absolute_pos = this->pos().xyz() + ( *itr )->rel_pos();
    unsigned int gh = this->realm()->encode_global_hull( absolute_pos.xy() );
    this->realm()->global_hulls.insert( gh );
  }
}

void UBoat::unregself()
{
  const MultiDef& md = multidef();
  for ( auto itr = md.hull.begin(), end = md.hull.end(); itr != end; ++itr )
  {
    Core::Pos3d absolute_pos = this->pos().xyz() + ( *itr )->rel_pos();
    unsigned int gh = this->realm()->encode_global_hull( absolute_pos.xy() );
    this->realm()->global_hulls.erase( gh );
  }
}

// navigable: Can the ship sit here?  ie is every point on the hull on water,and not blocked?
bool UBoat::navigable( const MultiDef& md, const Core::Pos4d& desired_pos )
{
  const Core::Vec2d r_min( md.minrx, md.minry );
  const Core::Vec2d r_max( md.maxrx, md.maxry );
  if ( !desired_pos.can_move_to( r_min ) || !desired_pos.can_move_to( r_max ) )
  {
#ifdef DEBUG_BOATS
    INFO_PRINT << "Location " << x << "," << y << " impassable, location is off the map\n";
#endif
    return false;
  }

  /* Test the external hull to make sure it's on water */
  for ( auto itr = md.hull.begin(), end = md.hull.end(); itr != end; ++itr )
  {
    Core::Pos3d absolute_pos = desired_pos.xyz() + ( *itr )->rel_pos();
#ifdef DEBUG_BOATS
    INFO_PRINT << "[" << ax << "," << ay << "]";
#endif
    /* See if any other ship hulls occupy this space */
    unsigned int gh = desired_pos.realm()->encode_global_hull( absolute_pos.xy() );
    if ( desired_pos.realm()->global_hulls.count( gh ) )  // already a boat there
    {
#ifdef DEBUG_BOATS
      INFO_PRINT << "Location " << realm->name() << " " << ax << "," << ay
                 << " already has a ship hull present\n";
#endif
      return false;
    }

    if ( !desired_pos.realm()->navigable( absolute_pos,
                                          Plib::systemstate.tile[( *itr )->objtype].height ) )
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
  return bc.stored_mdef.body_contains( obj->pos().xy() - bc.stored_pos.xy() );
}

// Helper function to move characters on boat. There's probably a better suited function somewhere
// else.
void _move_boat_mobile( Mobile::Character* chr, const Core::Pos4d& newpos,
                        const Multi::UBoat* boat )
{
  const Core::Pos4d oldpos = chr->pos();
  chr->lastxyz = oldpos.xyz();

  chr->setposition( newpos );
  // inhibits PropagateMove() in do_tellmoves() from sending packets to HSA clients
  chr->move_reason = Mobile::Character::MULTIMOVE;
  // needed also for offline chars because of the tracking of offline mobiles in realms
  MoveCharacterWorldPosition( oldpos, chr );

  // offline characters move with the boat but do nothing else, so we are done
  if ( !chr->logged_in() )
    return;

  // for npcs and online players, we need to let them update themselves (though the method does
  // nothing at this time?) - movement will be updated in do_tellmoves()
  chr->position_changed();

  if ( chr->client == nullptr )
    return;

  if ( oldpos.realm() != chr->realm() )
  {
    chr->realm_changed();  // moved here from uomod. Check why the two packets below are needed.
    Core::send_new_subserver( chr->client );
    Core::send_owncreate( chr->client, chr );
  }

  // TODO: check if those packets really belong here
  if ( chr->client->ClientType & Network::CLIENTTYPE_7090 )
  {
    Core::send_objects_newly_inrange_on_boat( chr->client, boat->serial );

    if ( chr->poisoned() )  // if poisoned send 0x17 for newer clients
      send_poisonhealthbar( chr->client, chr );

    if ( chr->invul() )  // if invul send 0x17 for newer clients
      send_invulhealthbar( chr->client, chr );
  }
  else
  {
    Core::send_goxyz( chr->client, chr );
    // lastx and lasty are set above so these two calls will work right.
    // FIXME these are also called, in this order, in MOVEMENT.CPP.
    // should be consolidated.
    Core::send_objects_newly_inrange_on_boat( chr->client, boat->serial );
  }
}

// Helper function to move items on boat. There's probably a better suited function somewhere
// else.
void _move_boat_item( Items::Item* item, const Core::Pos4d& newpos, const Multi::UBoat* /*boat*/ )
{
  const Core::Pos4d oldpos = item->pos();
  item->setposition( newpos );

  item->restart_decay_timer();  // Note: components don't decay, so I would expect this isn't a
                                // problem
  MoveItemWorldPosition( oldpos, item );

  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      item->pos(), RANGE_VISUAL, [&]( Mobile::Character* zonechr ) {
        Network::Client* client = zonechr->client;

        if ( !( client->ClientType & Network::CLIENTTYPE_7090 ) )
          send_item( client, item );
      } );

  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      oldpos, RANGE_VISUAL, [&]( Mobile::Character* zonechr ) {
        Network::Client* client = zonechr->client;

        // not in range.  If old loc was in range, send a delete.
        if ( !inrange( client->chr, item ) )
          send_remove_object( client, item );
      } );
}

void UBoat::move_travellers( const BoatContext& oldlocation )
{
  bool any_orphans = false;

  Core::Vec3d boatDelta = this->pos().xyz() - oldlocation.stored_pos.xyz();

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
    if ( obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      _move_boat_mobile( chr, Core::Pos4d( chr->pos().xyz() + boatDelta, this->realm() ), this );
    }
    else
    {
      Items::Item* item = static_cast<Items::Item*>( obj );
      _move_boat_item( item, Core::Pos4d( item->pos().xyz() + boatDelta, this->realm() ), this );
    }
  }

  if ( any_orphans )
    remove_orphans();
}

// TODO: the new facing can be given by (oldfacing + dir*2) & 7. But the order of the enum would
// then matter.
// TODO: the next two functions could be better taken care in position.h/vector.h
constexpr Core::UFACING _turn_towards( Core::UFACING oldfacing, UBoat::RELATIVE_DIR dir )
{
  return static_cast<Core::UFACING>( ( oldfacing + dir * 2 ) & 7 );
}
constexpr Core::Vec2d _turn_vector_around( const Core::Vec2d& delta, UBoat::RELATIVE_DIR dir )
{
  Core::Vec2d rotated_vec = delta;
  switch ( dir )
  {
  case UBoat::RELATIVE_DIR::LEFT:
    rotated_vec = Core::Vec2d( delta.y(), -delta.x() );
    break;
  case UBoat::RELATIVE_DIR::AROUND:
    rotated_vec = Core::Vec2d( -delta.x(), -delta.y() );
    break;
  case UBoat::RELATIVE_DIR::RIGHT:
    rotated_vec = Core::Vec2d( -delta.y(), delta.x() );
    break;
  case UBoat::RELATIVE_DIR::NO_TURN:
    break;
  }
  return rotated_vec;
}

Core::Pos4d UBoat::turn_traveller_coords( const Core::UObject* obj, RELATIVE_DIR dir ) const
{
  // rotate the relative distance of the object to the mast
  Core::Vec2d obj_rpos = _turn_vector_around( obj->pos().xy() - this->pos().xy(), dir );
  return this->pos() + obj_rpos;
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
    Core::Pos4d traveller_newpos = turn_traveller_coords( obj, dir );
    if ( obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      chr->facing = _turn_towards( static_cast<Core::UFACING>( chr->facing ), dir );
      _move_boat_mobile( chr, traveller_newpos, this );
    }
    else
    {
      Items::Item* item = static_cast<Items::Item*>( obj );
      _move_boat_item( item, traveller_newpos, this );
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
        _move_boat_mobile( chr, newpos, this );
        travellerRef.clear();
      }
    }
  }
  remove_orphans();
}


void UBoat::on_color_changed()
{
  // TODO: check this. It relied on the SHORT_MAX before to avoid some processing, but I think we
  // should just remove and re-add the boat at this location. I don't know if a multi can even
  // change colors.
  send_display_boat_to_inrange( this->pos() );
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

    if ( obj != nullptr )  // sometimes we've destroyed objects because of control scripts
    {
      if ( obj->ismobile() )
      {
        Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
        // dave 3/27/3, dont tell moves of offline PCs
        if ( chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) || chr->has_active_client() )
          chr->tellmove();
      }
    }
  }
}

bool UBoat::move_to( const Core::Pos4d& newpos, int flags )
{
  BoatContext oldstate( *this );
  BoatMoveGuard registerguard( this );
  bool result = false;

  if ( ( flags & Core::MOVEITEM_FORCELOCATION ) || navigable( multidef(), newpos ) )
  {
    set_dirty();

    setposition( newpos );
    move_multi_in_world( oldstate.stored_pos, this );

    move_travellers( oldstate );
    move_components();
    // NOTE, send_boat_to_inrange pauses those it sends to.
    send_display_boat_to_inrange( oldstate.stored_pos );
    // send_boat_to_inrange( this, oldx, oldy );
    do_tellmoves();
    unpause_paused();

    result = true;
  }

  return result;
}  // namespace Multi

bool UBoat::move( Core::UFACING dir, u8 speed, bool relative )
{
  bool result;

  BoatMoveGuard registerguard( this );

  Core::UFACING world_move_dir;   // relative to cardinal directions (world coordinates)
  Core::UFACING bowrelative_dir;  // relative to bow (0 means boat_facing() in world coordinates)

  if ( relative == false )
  {
    world_move_dir = dir;
    bowrelative_dir = static_cast<Core::UFACING>( ( dir + boat_facing() ) & 7 );
  }
  else
  {
    world_move_dir = static_cast<Core::UFACING>( ( dir + boat_facing() ) & 7 );
    bowrelative_dir = dir;
  }

  Core::Pos4d newpos = this->pos().move( world_move_dir );
  if ( navigable( multidef(), newpos ) )
  {
    BoatContext oldstate( *this );

    // TODO: update smooth_move to use the NEW positions of components and travellers
    send_smooth_move_to_inrange( bowrelative_dir, speed, newpos.xy() );

    set_dirty();

    setposition( newpos );
    move_multi_in_world( oldstate.stored_pos, this );

    move_travellers( oldstate );
    move_components();

    // TODO: merge smooth move and the updates below with send_display_boat_to_inrange()
    Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
        newpos, RANGE_VISUAL_LARGE_BUILDINGS, [&]( Mobile::Character* zonechr ) {
          Network::Client* client = zonechr->client;

          if ( client->ClientType & Network::CLIENTTYPE_7090 )
          {
            if ( !oldstate.stored_pos.inRange( client->chr->pos(), RANGE_VISUAL_LARGE_BUILDINGS ) )
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

    Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
        oldstate.stored_pos, RANGE_VISUAL_LARGE_BUILDINGS, [&]( Mobile::Character* zonechr ) {
          Network::Client* client = zonechr->client;

          if ( !newpos.inRange(
                   client->chr->pos(),
                   RANGE_VISUAL_LARGE_BUILDINGS ) )  // send remove to chrs only seeing the old loc
            send_remove_boat( client );
        } );

    do_tellmoves();
    unpause_paused();
    result = true;
  }
  else
  {
    result = false;
  }
  return result;
}

inline unsigned short UBoat::multiid_ifturn( RELATIVE_DIR dir )
{
  return ( multiid & ~3u ) | ( ( multiid + dir ) & 3 );
}

const MultiDef& UBoat::multi_ifturn( RELATIVE_DIR dir )
{
  unsigned short multiid_dir = multiid_ifturn( dir );
  passert( MultiDefByMultiIDExists( multiid_dir ) );
  return *MultiDefByMultiID( multiid_dir );
}

Core::UFACING UBoat::boat_facing() const
{
  return static_cast<Core::UFACING>( ( multiid & 3 ) * 2 );
}

const BoatShape& UBoat::boatshape() const
{
  passert( Core::gamestate.boatshapes.count( multiid ) != 0 );
  return *Core::gamestate.boatshapes[multiid];
}

void _log_badcomponent( const Items::Item* item )
{
  u32 containerSerial = ( item->container != nullptr ) ? item->container->serial : 0;
  POLLOG_ERROR.Format(
      "Boat component is gotten or in a container and couldn't be moved together with the "
      "boat: serial 0x{:X}\n, graphic: 0x{:X}, container: 0x{:X}." )
      << item->serial << item->graphic << containerSerial;
}

bool UBoat::is_plank( const Items::Item* item )
{
  return ( item->objtype_ == Core::settingsManager.extobj.port_plank ||
           item->objtype_ == Core::settingsManager.extobj.starboard_plank );
}

void UBoat::transform_components( const BoatShape& old_boatshape )
{
  const BoatShape& bshape = boatshape();

  auto itr = Components.begin();
  auto end = Components.end();

  auto itr2 = bshape.Componentshapes.begin();
  auto end2 = bshape.Componentshapes.end();

  auto old_itr = old_boatshape.Componentshapes.begin();
  auto old_end = old_boatshape.Componentshapes.end();

  for ( ; itr != end && itr2 != end2 && old_itr != old_end; ++itr, ++itr2, ++old_itr )
  {
    Items::Item* item = itr->get();
    if ( item == nullptr || item->orphan() )
      continue;

    // POL would crash if the component is no longer in the world.
    // This can only happen if someone forces the item out of the boat, so log it.
    if ( item->container != nullptr || item->has_gotten_by() )
    {
      _log_badcomponent( item );
      continue;
    }

    item->set_dirty();
    // keep planks open while turning
    if ( is_plank( item ) && item->graphic == old_itr->altgraphic )
      item->graphic = itr2->altgraphic;
    else
      item->graphic = itr2->graphic;

    Core::Pos4d newpos = ( this->pos() + itr2->rel_pos() );
    _move_boat_item( item, newpos, this );
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
    if ( item == nullptr || item->orphan() )
      continue;

    // POL would crash if the component is no longer in the world.
    // This can only happen if someone forces the item out of the boat, so log it.
    if ( item->container != nullptr || item->has_gotten_by() )
    {
      _log_badcomponent( item );
      continue;
    }

    item->set_dirty();
    Core::Pos4d newpos = this->pos() + itr2->rel_pos();
    _move_boat_item( item, newpos, this );
  }
}

bool UBoat::turn( RELATIVE_DIR dir )
{
  bool result = false;
  BoatMoveGuard registerguard( this );

  const MultiDef& newmd = multi_ifturn( dir );

  if ( navigable( newmd, this->pos() ) )
  {
    BoatContext bc( *this );
    const BoatShape& old_boatshape = boatshape();

    set_dirty();
    multiid = multiid_ifturn( dir );

    turn_travellers( dir, bc );
    transform_components( old_boatshape );
    send_display_boat_to_inrange( this->pos() );
    do_tellmoves();
    unpause_paused();
    facing = _turn_towards( static_cast<Core::UFACING>( facing ), dir );

    result = true;
  }
  else
  {
    result = false;
  }
  return result;
}

void UBoat::register_object( UObject* obj )
{
  if ( find( travellers_.begin(), travellers_.end(), obj ) == travellers_.end() )
  {
    set_dirty();
    travellers_.push_back( Traveller( obj ) );
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
  if ( Core::settingsManager.polvar.DataWrittenBy < 99 )
  {
    passert_always_r( graphic >= 0x4000, "Unexpected boat graphic < 0x4000 in POL < 099 data" );
    multiid = graphic - 0x4000;
  }
  base::fixInvalidGraphic();
}

void UBoat::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );

  // POL098 and earlier was not saving a MultiID in its data files,
  // but it was using 0x4000 + id as graphic instead. Not respecting
  // this would rotate most of the boats during POL098 -> POL99 migration
  if ( Core::settingsManager.polvar.DataWrittenBy >= 99 )
    multiid = elem.remove_ushort( "MultiID", this->multidef().multiid );

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
          Components.push_back( Component( item ) );
        }
        else if ( on_ship( bc, item ) )
        {
          travellers_.push_back( Traveller( item ) );
        }
      }
    }
    else
    {
      Mobile::Character* chr = Core::system_find_mobile( tmp_serial );

      if ( chr != nullptr )
      {
        if ( on_ship( bc, chr ) )
          travellers_.push_back( Traveller( chr ) );
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
        Components.push_back( Component( item ) );
      }
    }
  }
  reread_components();
  rescan_components();

  regself();  // do this after our x,y are known.
  // consider throwing if starting position isn't passable.

  Core::start_script( "misc/boat", make_boatref( this ) );
}

void UBoat::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );

  sw() << "\tMultiID\t" << multiid << pf_endl;

  BoatContext bc( *this );

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();
    if ( !obj->orphan() && on_ship( bc, obj ) )
    {
      sw() << "\tTraveller\t0x" << fmt::hex( obj->serial ) << pf_endl;
    }
  }
  for ( auto& component : Components )
  {
    if ( component != nullptr && !component->orphan() )
    {
      sw() << "\tComponent\t0x" << fmt::hex( component->serial ) << pf_endl;
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
  boat->multiid = multiid;
  boat->serial = Core::GetNewItemSerialNumber();
  boat->serial_ext = ctBEu32( boat->serial );

  boat->facing = facing;

  boat->setposition( pos );
  add_multi_to_world( boat );
  boat->send_display_boat_to_inrange( boat->pos() );

  boat->create_components();
  boat->rescan_components();

  unpause_paused();
  boat->regself();

  ////hash
  Core::objStorageManager.objecthash.Insert( boat );
  ////

  Core::start_script( "misc/boat", make_boatref( boat ) );
  return make_boatref( boat );
}

void UBoat::create_components()
{
  const BoatShape& bshape = boatshape();
  for ( std::vector<BoatShape::ComponentShape>::const_iterator itr = bshape.Componentshapes.begin(),
                                                               end = bshape.Componentshapes.end();
        itr != end; ++itr )
  {
    Items::Item* component = Items::Item::create( itr->objtype );
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

    component->disable_decay();

    component->setposition( this->pos() + itr->rel_pos() );
    component->movable( false );
    add_item_to_world( component );
    update_item_to_inrange( component );
    Components.push_back( Component( component ) );
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
                + 3 * sizeof( Traveller* ) + travellers_.capacity() * sizeof( Traveller ) +
                3 * sizeof( Items::Item** ) + Components.capacity() * sizeof( Items::Item* );
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

  Core::WorldIterator<Core::OnlinePlayerFilter>::InVisualRange(
      boat,
      [&]( Mobile::Character* zonechr ) { Core::send_remove_object( zonechr->client, boat ); } );
  remove_multi_from_world( boat );
  boat->destroy();
  return new Bscript::BLong( 1 );
}
}  // namespace Multi
}  // namespace Pol
