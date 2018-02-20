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
#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/streamsaver.h"
#include "../../plib/systemstate.h"
#include "../containr.h"
#include "../extobj.h"
#include "../fnsearch.h"
#include "../globals/object_storage.h"
#include "../globals/settings.h"
#include "../globals/uvars.h"
#include "../item/item.h"
#include "../item/itemdesc.h"
#include "../mdelta.h"
#include "../mkscrobj.h"
#include "../mobile/charactr.h"
#include "../network/client.h"
#include "../network/packethelper.h"
#include "../network/packets.h"
#include "../pktdef.h"
#include "../polvar.h"
#include "../realms/realm.h"
#include "../scrsched.h"
#include "../tiles.h"
#include "../uconst.h"
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
    graphic = static_cast<unsigned short>( strtoul( tmp.c_str(), NULL, 0 ) );
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
    graphic = static_cast<unsigned short>( strtoul( tmp.c_str(), NULL, 0 ) );
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
    altgraphic = static_cast<unsigned short>( strtoul( alttmp.c_str(), NULL, 0 ) );
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
    if ( iter->second != NULL )
      delete iter->second;
    iter->second = NULL;
  }
  Core::gamestate.boatshapes.clear();
}

bool BoatShapeExists( u16 multiid )
{
  return Core::gamestate.boatshapes.count( multiid ) != 0;
}

void UBoat::send_smooth_move( Network::Client* client, Core::UFACING move_dir, u8 speed, u16 newx,
                              u16 newy, bool relative )
{
  Network::PktHelper::PacketOut<Network::PktOut_F6> msg;

  u16 xmod = newx - x;
  u16 ymod = newy - y;
  Core::UFACING b_facing = boat_facing();

  if ( relative == false )
    move_dir = static_cast<Core::UFACING>( ( b_facing + move_dir ) * 7 );

  msg->offset += 2;  // Length
  msg->Write<u32>( serial_ext );
  msg->Write<u8>( speed );

  msg->Write<u8>( move_dir );
  msg->Write<u8>( b_facing );

  msg->WriteFlipped<u16>( newx );
  msg->WriteFlipped<u16>( newy );
  msg->WriteFlipped<u16>( ( z < 0 ) ? static_cast<u16>( 0x10000 + z ) : static_cast<u16>( z ) );

  u16 object_count = static_cast<u16>( travellers_.size() + Components.size() );

  msg->WriteFlipped<u16>( object_count );

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    if ( !obj->orphan() )
    {
      msg->Write<u32>( obj->serial_ext );
      msg->WriteFlipped<u16>( static_cast<u16>( obj->x + xmod ) );
      msg->WriteFlipped<u16>( static_cast<u16>( obj->y + ymod ) );
      msg->WriteFlipped<u16>(
          static_cast<u16>( ( obj->z < 0 ) ? ( 0x10000 + obj->z ) : ( obj->z ) ) );
    }
  }

  for ( auto& component : Components )
  {
    if ( component != NULL && !component->orphan() )
    {
      msg->Write<u32>( component->serial_ext );
      msg->WriteFlipped<u16>( static_cast<u16>( component->x + xmod ) );
      msg->WriteFlipped<u16>( static_cast<u16>( component->y + ymod ) );
      msg->WriteFlipped<u16>( static_cast<u16>( ( component->z < 0 ) ? ( 0x10000 + component->z )
                                                                     : ( component->z ) ) );
    }
  }

  u16 len = msg->offset;

  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  msg.Send( client, len );
}

void UBoat::send_smooth_move_to_inrange( Core::UFACING move_dir, u8 speed, u16 newx, u16 newy,
                                         bool relative )
{
  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      newx, newy, realm, RANGE_VISUAL_LARGE_BUILDINGS, [&]( Mobile::Character* zonechr ) {
        Network::Client* client = zonechr->client;

        if ( inrange( client->chr, this ) &&
             client->ClientType & Network::CLIENTTYPE_7090 )  // send this only to those who see the
                                                              // old location aswell
          send_smooth_move( client, move_dir, speed, newx, newy, relative );
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
  msg->WriteFlipped<u16>( this->x );
  msg->WriteFlipped<u16>( this->y );
  msg->Write<s8>( this->z );
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

      msg->WriteFlipped<u16>( obj->x );
      msg->WriteFlipped<u16>( obj->y );
      msg->Write<s8>( obj->z );
      msg->Write<u8>( obj->facing );
      msg->WriteFlipped<u16>( obj->color );

      msg->Write<u8>( flags );  // FLAGS
      msg->offset += 2;  // HSA access flags, TODO find out what these are for and implement it
    }
  }

  for ( auto& component : Components )
  {
    if ( component != NULL && !component->orphan() )
    {
      msg->Write<u8>( 0xF3u );
      msg->WriteFlipped<u16>( 0x1u );
      msg->Write<u8>( 0x0u );  // ItemData flag
      msg->Write<u32>( component->serial_ext );
      msg->WriteFlipped<u16>( component->graphic );
      msg->offset++;  // ID offset, TODO CHECK IF NEED THESE
      msg->WriteFlipped<u16>( component->get_senditem_amount() );  // Amount
      msg->WriteFlipped<u16>( component->get_senditem_amount() );  // Amount
      msg->WriteFlipped<u16>( component->x );
      msg->WriteFlipped<u16>( component->y );
      msg->Write<s8>( component->z );
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
  msg->WriteFlipped<u16>( x );
  msg->WriteFlipped<u16>( y );
  msg->Write<s8>( z );
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
    if ( component != NULL && !component->orphan() )
      send_item( client, component.get() );
  }
}

void UBoat::send_display_boat_to_inrange( u16 oldx, u16 oldy )
{
  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      x, y, realm, RANGE_VISUAL_LARGE_BUILDINGS, [&]( Mobile::Character* zonechr ) {
        Network::Client* client = zonechr->client;

        if ( client->ClientType & Network::CLIENTTYPE_7090 )
          send_display_boat( client );
        else if ( client->ClientType & Network::CLIENTTYPE_7000 )
          send_boat( client );
        else
          send_boat_old( client );
      } );

  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      oldx, oldy, this->realm, RANGE_VISUAL_LARGE_BUILDINGS, [&]( Mobile::Character* zonechr ) {
        Network::Client* client = zonechr->client;

        if ( !inrange( client->chr, this ) )  // send remove to chrs only seeing the old loc
          send_remove_boat( client );
      } );
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
  msg2->WriteFlipped<u16>( this->x );
  msg2->WriteFlipped<u16>( this->y );
  msg2->Write<s8>( this->z );
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
  msg->WriteFlipped<u16>( this->x );
  msg->WriteFlipped<u16>( this->y );
  msg->Write<s8>( this->z );
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
  tillerman = NULL;
  hold = NULL;
  portplank = NULL;
  starboardplank = NULL;
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
  for ( MultiDef::HullList::const_iterator itr = md.hull.begin(), end = md.hull.end(); itr != end;
        ++itr )
  {
    unsigned short ax = x + ( *itr )->x;
    unsigned short ay = y + ( *itr )->y;

    unsigned int gh = realm->encode_global_hull( ax, ay );
    realm->global_hulls.insert( gh );
  }
}

void UBoat::unregself()
{
  const MultiDef& md = multidef();
  for ( MultiDef::HullList::const_iterator itr = md.hull.begin(), end = md.hull.end(); itr != end;
        ++itr )
  {
    unsigned short ax = x + ( *itr )->x;
    unsigned short ay = y + ( *itr )->y;

    unsigned int gh = realm->encode_global_hull( ax, ay );
    realm->global_hulls.erase( gh );
  }
}

// navigable: Can the ship sit here?  ie is every point on the hull on water,and not blocked?
bool UBoat::navigable( const MultiDef& md, unsigned short x, unsigned short y, short z,
                       Realms::Realm* realm )
{
  if ( int( x + md.minrx ) < 0 || int( x + md.maxrx ) > int( realm->width() ) ||
       int( y + md.minry ) < 0 || int( y + md.maxry ) > int( realm->height() ) )
  {
#ifdef DEBUG_BOATS
    INFO_PRINT << "Location " << x << "," << y << " impassable, location is off the map\n";
#endif
    return false;
  }

  /* Test the external hull to make sure it's on water */

  for ( MultiDef::HullList::const_iterator itr = md.hull.begin(), end = md.hull.end(); itr != end;
        ++itr )
  {
    unsigned short ax = x + ( *itr )->x;
    unsigned short ay = y + ( *itr )->y;
    short az = z + ( *itr )->z;
#ifdef DEBUG_BOATS
    INFO_PRINT << "[" << ax << "," << ay << "]";
#endif
    /*
     * See if any other ship hulls occupy this space
     */
    unsigned int gh = realm->encode_global_hull( ax, ay );
    if ( realm->global_hulls.count( gh ) )  // already a boat there
    {
#ifdef DEBUG_BOATS
      INFO_PRINT << "Location " << realm->name() << " " << ax << "," << ay
                 << " already has a ship hull present\n";
#endif
      return false;
    }

    if ( !realm->navigable( ax, ay, az, Plib::systemstate.tile[( *itr )->objtype].height ) )
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
    if ( item->container != NULL )
      return false;
  }
  short rx = obj->x - bc.x;
  short ry = obj->y - bc.y;

  return bc.mdef.body_contains( rx, ry );
}

void UBoat::move_travellers( Core::UFACING move_dir, const BoatContext& oldlocation,
                             unsigned short newx, unsigned short newy, Realms::Realm* oldrealm )
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
      ;
      continue;
    }

    obj->set_dirty();
    if ( obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );

      if ( chr->logged_in() )
      {
        chr->lastx = chr->x;
        chr->lasty = chr->y;

        if ( newx != USHRT_MAX &&
             newy != USHRT_MAX )  // dave added 3/27/3, if move_xy was used, dont use facing
        {
          s16 dx, dy;
          dx = chr->x - oldlocation.x;  // keeps relative distance from boat mast
          dy = chr->y - oldlocation.y;
          chr->x = newx + dx;
          chr->y = newy + dy;
        }
        else
        {
          chr->x += Core::move_delta[move_dir].xmove;
          chr->y += Core::move_delta[move_dir].ymove;
        }

        MoveCharacterWorldPosition( chr->lastx, chr->lasty, chr->x, chr->y, chr, oldrealm );
        chr->position_changed();
        if ( chr->client != NULL )
        {
          if ( oldrealm != chr->realm )
          {
            Core::send_new_subserver( chr->client );
            Core::send_owncreate( chr->client, chr );
          }

          if ( chr->client->ClientType & Network::CLIENTTYPE_7090 )
          {
            Core::send_objects_newly_inrange_on_boat( chr->client, this->serial );

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
            Core::send_objects_newly_inrange_on_boat( chr->client, this->serial );
          }
        }
        chr->move_reason = Mobile::Character::MULTIMOVE;
      }
      else
      {
        // characters that are logged out move with the boat
        // they aren't in the worldzones so this is real easy.
        chr->lastx = chr->x;  // I think in this case setting last? isn't
        chr->lasty = chr->y;  // necessary, but I'll do it anyway.

        if ( newx != USHRT_MAX &&
             newy != USHRT_MAX )  // dave added 3/27/3, if move_xy was used, dont use facing
        {
          s16 dx, dy;
          dx = chr->x - oldlocation.x;  // keeps relative distance from boat mast
          dy = chr->y - oldlocation.y;
          chr->x = newx + dx;
          chr->y = newy + dy;
        }
        else
        {
          chr->x += Core::move_delta[move_dir].xmove;
          chr->y += Core::move_delta[move_dir].ymove;
        }
      }
    }
    else
    {
      Items::Item* item = static_cast<Items::Item*>( obj );

      u16 oldx, oldy;

      if ( newx != USHRT_MAX &&
           newy != USHRT_MAX )  // dave added 4/9/3, if move_xy was used, dont use facing
      {
        s16 dx, dy;
        dx = item->x - oldlocation.x;  // keeps relative distance from boat mast
        dy = item->y - oldlocation.y;
        // Core::move_item( item, newx + dx, newy + dy, item->z, NULL );

        item->set_dirty();

        oldx = item->x;
        oldy = item->y;

        item->x = newx + dx;
        item->y = newy + dy;

        item->restart_decay_timer();
        MoveItemWorldPosition( oldx, oldy, item, oldrealm );
      }
      else
      {
        item->set_dirty();

        oldx = item->x;
        oldy = item->y;

        item->x += Core::move_delta[move_dir].xmove;
        item->y += Core::move_delta[move_dir].ymove;

        item->restart_decay_timer();
        MoveItemWorldPosition( oldx, oldy, item, NULL );
      }

      Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
          item->x, item->y, realm, RANGE_VISUAL, [&]( Mobile::Character* zonechr ) {
            Network::Client* client = zonechr->client;

            if ( !( client->ClientType & Network::CLIENTTYPE_7090 ) )
              send_item( client, item );
          } );

      Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
          oldx, oldy, oldrealm, RANGE_VISUAL, [&]( Mobile::Character* zonechr ) {
            Network::Client* client = zonechr->client;

            if ( !inrange( client->chr,
                           item ) )  // not in range.  If old loc was in range, send a delete.
              send_remove_object( client, item );
          } );
    }
  }

  if ( any_orphans )
    remove_orphans();
}

void UBoat::turn_traveller_coords( Mobile::Character* chr, RELATIVE_DIR dir )
{
  chr->lastx = chr->x;
  chr->lasty = chr->y;

  s16 xd = chr->x - x;
  s16 yd = chr->y - y;

  switch ( dir )
  {
  case LEFT:
    chr->x = x + yd;
    chr->y = y - xd;
    chr->facing = static_cast<Core::UFACING>( ( chr->facing + 6 ) & 7 );
    break;
  case AROUND:
    chr->x = x - xd;
    chr->y = y - yd;
    chr->facing = static_cast<Core::UFACING>( ( chr->facing + 4 ) & 7 );
    break;
  case RIGHT:
    chr->x = x - yd;
    chr->y = y + xd;
    chr->facing = static_cast<Core::UFACING>( ( chr->facing + 2 ) & 7 );
    break;
  case NO_TURN:
    break;
  }
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
    if ( obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( chr->logged_in() )
      {
        // send_remove_character_to_nearby( chr );


        turn_traveller_coords( chr, dir );


        Core::MoveCharacterWorldPosition( chr->lastx, chr->lasty, chr->x, chr->y, chr, NULL );
        chr->position_changed();
        if ( chr->client != NULL )
        {
          if ( chr->client->ClientType & Network::CLIENTTYPE_7090 )
          {
            if ( chr->poisoned() )  // if poisoned send 0x17 for newer clients
              send_poisonhealthbar( chr->client, chr );

            if ( chr->invul() )  // if invul send 0x17 for newer clients
              send_invulhealthbar( chr->client, chr );

            Core::send_objects_newly_inrange_on_boat( chr->client, this->serial );
          }
          else
          {
            Core::send_goxyz( chr->client, chr );
            // lastx and lasty are set above so these two calls will work right.
            // FIXME these are also called, in this order, in MOVEMENT.CPP.
            // should be consolidated.
            Core::send_objects_newly_inrange_on_boat( chr->client, this->serial );
          }
        }
        chr->move_reason = Mobile::Character::MULTIMOVE;
        // chr->lastx = ~ (unsigned short) 0; // force tellmove() to send "owncreate" and not send
        // deletes.
        // chr->lasty = ~ (unsigned short) 0;
      }
      else
      {
        turn_traveller_coords( chr, dir );
      }
    }
    else
    {
      Items::Item* item = static_cast<Items::Item*>( obj );
      s16 xd = item->x - x;
      s16 yd = item->y - y;
      u16 newx( 0 );
      u16 newy( 0 );
      switch ( dir )
      {
      case NO_TURN:
        newx = item->x;
        newy = item->y;
        break;
      case LEFT:
        newx = x + yd;
        newy = y - xd;
        break;
      case AROUND:
        newx = x - xd;
        newy = y - yd;
        break;
      case RIGHT:
        newx = x - yd;
        newy = y + xd;
        break;
      }
      item->set_dirty();

      u16 oldx = item->x;
      u16 oldy = item->y;

      item->x = newx;
      item->y = newy;

      item->restart_decay_timer();

      MoveItemWorldPosition( oldx, oldy, item, NULL );

      Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
          item->x, item->y, realm, RANGE_VISUAL, [&]( Mobile::Character* zonechr ) {
            Network::Client* client = zonechr->client;

            if ( !( client->ClientType & Network::CLIENTTYPE_7090 ) )
              send_item( client, item );
          } );

      Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
          oldx, oldy, realm, RANGE_VISUAL, [&]( Mobile::Character* zonechr ) {
            Network::Client* client = zonechr->client;

            if ( !inrange( client->chr,
                           item ) )  // not in range.  If old loc was in range, send a delete.
              send_remove_object( client, item );
          } );
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
      if ( obj == NULL )
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

void UBoat::move_offline_mobiles( Core::xcoord new_x, Core::ycoord new_y, Core::zcoord new_z,
                                  Realms::Realm* new_realm )
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
        chr->x = new_x;
        chr->y = new_y;
        chr->z = static_cast<signed char>( new_z );
        chr->realm = new_realm;
        chr->realm_changed();  // not sure if neccessary...
        travellerRef.clear();
      }
    }
  }
  remove_orphans();
}

void UBoat::adjust_traveller_z( s8 delta_z )
{
  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();
    obj->z += delta_z;
  }
  for ( auto& component : Components )
  {
    component->z += delta_z;
  }
}

void UBoat::on_color_changed()
{
  send_display_boat_to_inrange();
}

void UBoat::realm_changed()
{
  BoatContext bc( *this );

  for ( auto& travellerRef : travellers_ )
  {
    UObject* obj = travellerRef.get();

    if ( !obj->orphan() && on_ship( bc, obj ) && obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      Core::send_remove_character_to_nearby( chr );
      chr->realm = realm;
      chr->realm_changed();
    }
    if ( !obj->orphan() && on_ship( bc, obj ) && Core::IsItem( obj->serial ) )
    {
      Items::Item* item = static_cast<Items::Item*>( obj );
      item->realm = realm;
      if ( item->isa( Core::UOBJ_CLASS::CLASS_CONTAINER ) )
      {
        Core::UContainer* cont = static_cast<Core::UContainer*>( item );
        cont->for_each_item( Core::setrealm, (void*)realm );
      }
    }
  }
  for ( auto& component : Components )
  {
    component->realm = realm;
  }
}

bool UBoat::deck_empty() const
{
  return travellers_.empty();
}

bool UBoat::hold_empty() const
{
  Items::Item* it = this->hold;
  if ( it == NULL )
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

    if ( obj != NULL )  // sometimes we've destroyed objects because of control scripts
    {
      if ( obj->ismobile() )
      {
        Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
        if ( chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) ||
             chr->has_active_client() )  // dave 3/27/3, dont tell moves of offline PCs
          chr->tellmove();
      }
    }
  }
}

// dave 3/26/3 added
bool UBoat::move_xy( unsigned short newx, unsigned short newy, int flags, Realms::Realm* oldrealm )
{
  bool result;
  BoatMoveGuard registerguard( this );

  if ( ( flags & Core::MOVEITEM_FORCELOCATION ) || navigable( multidef(), newx, newy, z, realm ) )
  {
    BoatContext bc( *this );

    set_dirty();
    move_multi_in_world( x, y, newx, newy, this, oldrealm );

    u16 oldx = x;
    u16 oldy = y;
    x = newx;
    y = newy;

    move_travellers( Core::FACING_N, bc, newx, newy,
                     oldrealm );  // facing is ignored if params 3 & 4 are not USHRT_MAX
    move_components( oldrealm );
    // NOTE, send_boat_to_inrange pauses those it sends to.
    send_display_boat_to_inrange( oldx, oldy );
    // send_boat_to_inrange( this, oldx, oldy );
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

bool UBoat::move( Core::UFACING dir, u8 speed, bool relative )
{
  bool result;

  BoatMoveGuard registerguard( this );

  Core::UFACING move_dir;

  if ( relative == false )
    move_dir = dir;
  else
    move_dir = static_cast<Core::UFACING>( ( dir + boat_facing() ) & 7 );

  unsigned short newx, newy;
  newx = x + Core::move_delta[move_dir].xmove;
  newy = y + Core::move_delta[move_dir].ymove;

  if ( navigable( multidef(), newx, newy, z, realm ) )
  {
    BoatContext bc( *this );

    send_smooth_move_to_inrange( move_dir, speed, newx, newy, relative );

    set_dirty();

    move_multi_in_world( x, y, newx, newy, this, realm );

    u16 oldx = x;
    u16 oldy = y;
    x = newx;
    y = newy;

    // NOTE, send_boat_to_inrange pauses those it sends to.
    // send_boat_to_inrange( this, oldx, oldy );
    move_travellers( move_dir, bc, x, y, realm );
    move_components( realm );

    Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
        x, y, realm, RANGE_VISUAL_LARGE_BUILDINGS, [&]( Mobile::Character* zonechr ) {
          Network::Client* client = zonechr->client;

          if ( client->ClientType & Network::CLIENTTYPE_7090 )
          {
            if ( Core::inrange( client->chr->x, client->chr->y, oldx, oldy ) )
              return;
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

    Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
        oldx, oldy, realm, RANGE_VISUAL_LARGE_BUILDINGS, [&]( Mobile::Character* zonechr ) {
          Network::Client* client = zonechr->client;

          if ( !inrange( client->chr, this ) )  // send remove to chrs only seeing the old loc
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


void UBoat::transform_components( const BoatShape& old_boatshape, Realms::Realm* oldrealm )
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
    if ( item != NULL )
    {
      if ( item->orphan() )
        continue;

      // This should be rare enough for a simple log to be the solution. We don't want POL to crash
      // in MoveItemWorldPosition() because the item was not in the world to start with, so we skip
      // it.
      if ( item->container != NULL || item->has_gotten_by() )
      {
        u32 containerSerial = ( item->container != NULL ) ? item->container->serial : 0;
        POLLOG_ERROR.Format(
            "Boat component is gotten or in a container and couldn't be moved together with the "
            "boat: serial 0x{:X}\n, graphic: 0x{:X}, container: 0x{:X}." )
            << item->serial << item->graphic << containerSerial;
        continue;
      }

      item->set_dirty();
      if ( item->objtype_ == Core::settingsManager.extobj.port_plank &&
           item->graphic == old_itr->altgraphic )
        item->graphic = itr2->altgraphic;
      else if ( item->objtype_ == Core::settingsManager.extobj.starboard_plank &&
                item->graphic == old_itr->altgraphic )
        item->graphic = itr2->altgraphic;
      else
        item->graphic = itr2->graphic;

      u16 oldx = item->x;
      u16 oldy = item->y;

      item->x = x + itr2->xdelta;
      item->y = y + itr2->ydelta;
      item->z = z + static_cast<s8>( itr2->zdelta );

      MoveItemWorldPosition( oldx, oldy, item, oldrealm );

      Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
          item->x, item->y, realm, RANGE_VISUAL, [&]( Mobile::Character* zonechr ) {
            Network::Client* client = zonechr->client;

            if ( !( client->ClientType & Network::CLIENTTYPE_7090 ) )
              send_item( client, item );
          } );

      Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
          oldx, oldy, oldrealm, RANGE_VISUAL, [&]( Mobile::Character* zonechr ) {
            Network::Client* client = zonechr->client;

            if ( !inrange( client->chr,
                           item ) )  // not in range.  If old loc was in range, send a delete.
              send_remove_object( client, item );
          } );
    }
  }
}

void UBoat::move_components( Realms::Realm* oldrealm )
{
  const BoatShape& bshape = boatshape();
  auto itr = Components.begin();
  auto end = Components.end();
  auto itr2 = bshape.Componentshapes.begin();
  auto end2 = bshape.Componentshapes.end();
  for ( ; itr != end && itr2 != end2; ++itr, ++itr2 )
  {
    Items::Item* item = itr->get();
    if ( item != NULL )
    {
      if ( item->orphan() )
      {
        continue;
      }

      // This should be rare enough for a simple log to be the solution. We don't want POL to crash
      // in MoveItemWorldPosition() because the item was not in the world to start with, so we skip
      // it.
      if ( item->container != NULL || item->has_gotten_by() )
      {
        u32 containerSerial = ( item->container != NULL ) ? item->container->serial : 0;
        POLLOG_INFO.Format(
            "Boat component is gotten or in a container and couldn't be moved together with the "
            "boat: serial 0x{:X}\n, graphic: 0x{:X}, container: 0x{:X}." )
            << item->serial << item->graphic << containerSerial;
        continue;
      }

      item->set_dirty();

      u16 oldx = item->x;
      u16 oldy = item->y;

      item->x = x + itr2->xdelta;
      item->y = y + itr2->ydelta;
      item->z = z + static_cast<s8>( itr2->zdelta );

      MoveItemWorldPosition( oldx, oldy, item, oldrealm );

      Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
          item->x, item->y, realm, RANGE_VISUAL, [&]( Mobile::Character* zonechr ) {
            Network::Client* client = zonechr->client;

            if ( !( client->ClientType & Network::CLIENTTYPE_7090 ) )
              send_item( client, item );
          } );

      Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
          oldx, oldy, oldrealm, RANGE_VISUAL, [&]( Mobile::Character* zonechr ) {
            Network::Client* client = zonechr->client;

            if ( !inrange( client->chr,
                           item ) )  // not in range.  If old loc was in range, send a delete.
              send_remove_object( client, item );
          } );
    }
  }
}

bool UBoat::turn( RELATIVE_DIR dir )
{
  bool result;
  BoatMoveGuard registerguard( this );

  const MultiDef& newmd = multi_ifturn( dir );

  if ( navigable( newmd, x, y, z, realm ) )
  {
    BoatContext bc( *this );
    const BoatShape& old_boatshape = boatshape();

    set_dirty();
    multiid = multiid_ifturn( dir );

    // send_boat_to_inrange( this, x ,y, false ); // pauses those it sends to
    turn_travellers( dir, bc );
    transform_components( old_boatshape, NULL );
    send_display_boat_to_inrange( x, y );
    do_tellmoves();
    unpause_paused();
    facing = ( ( dir * 2 ) + facing ) & 7;
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

  if ( portplank != NULL && !portplank->orphan() )
  {
    plank = static_cast<UPlank*>( portplank );
    plank->setboat( this );
  }

  if ( starboardplank != NULL && !starboardplank->orphan() )
  {
    plank = static_cast<UPlank*>( starboardplank );
    plank->setboat( this );
  }
}

void UBoat::reread_components()
{
  for ( auto& component : Components )
  {
    if ( component == NULL )
      continue;
    // check boat members here
    if ( component->objtype_ == Core::settingsManager.extobj.tillerman && tillerman == NULL )
      tillerman = component.get();
    if ( component->objtype_ == Core::settingsManager.extobj.port_plank && portplank == NULL )
      portplank = component.get();
    if ( component->objtype_ == Core::settingsManager.extobj.starboard_plank &&
         starboardplank == NULL )
      starboardplank = component.get();
    if ( component->objtype_ == Core::settingsManager.extobj.hold && hold == NULL )
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
      if ( item != NULL )
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

      if ( chr != NULL )
      {
        if ( on_ship( bc, chr ) )
          travellers_.push_back( Traveller( chr ) );
      }
    }
  }
  while ( elem.remove_prop( "Component", &tmp_serial ) )
  {
    Items::Item* item = Core::system_find_item( tmp_serial );
    if ( item != NULL )
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
    if ( component != NULL && !component->orphan() )
    {
      sw() << "\tComponent\t0x" << fmt::hex( component->serial ) << pf_endl;
    }
  }
}

Bscript::BObjectImp* UBoat::scripted_create( const Items::ItemDesc& descriptor, u16 x, u16 y, s8 z,
                                             Realms::Realm* realm, int flags )
{
  unsigned short multiid = descriptor.multiid;
  unsigned short multiid_offset =
      static_cast<unsigned short>( ( flags & CRMULTI_FACING_MASK ) >> CRMULTI_FACING_SHIFT );
  unsigned char facing = static_cast<unsigned char>( multiid_offset * 2 );
  multiid += multiid_offset;

  const MultiDef* md = MultiDefByMultiID( multiid );
  if ( md == NULL )
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

  if ( !navigable( *md, x, y, z, realm ) )
  {
    return new Bscript::BError( "Position indicated is impassable" );
  }

  UBoat* boat = new UBoat( descriptor );
  boat->multiid = multiid;
  boat->serial = Core::GetNewItemSerialNumber();
  boat->serial_ext = ctBEu32( boat->serial );
  boat->x = x;
  boat->y = y;
  boat->z = z;
  boat->realm = realm;
  boat->facing = facing;
  add_multi_to_world( boat );
  boat->send_display_boat_to_inrange( x, y );
  // send_boat_to_inrange( boat, x, y, false );
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
    if ( component == NULL )
      continue;
    // check boat members here
    if ( component->objtype_ == Core::settingsManager.extobj.tillerman && tillerman == NULL )
      tillerman = component;
    if ( component->objtype_ == Core::settingsManager.extobj.port_plank && portplank == NULL )
      portplank = component;
    if ( component->objtype_ == Core::settingsManager.extobj.starboard_plank &&
         starboardplank == NULL )
      starboardplank = component;
    if ( component->objtype_ == Core::settingsManager.extobj.hold && hold == NULL )
      hold = component;

    component->graphic = itr->graphic;
    // component itemdesc entries generally have graphic=1, so they don't get their height set.
    component->height = Core::tileheight( component->graphic );
    component->x = x + itr->xdelta;
    component->y = y + itr->ydelta;
    component->z = z + static_cast<s8>( itr->zdelta );
    component->disable_decay();
    component->movable( false );
    component->realm = realm;
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
    if ( component != NULL && !component->orphan() )
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
    if ( component != NULL && !component->orphan() )
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
}
}
