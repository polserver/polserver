/** @file
 *
 * @par History
 * - 2005/05/28 Shinigami: now u can call open_trade_window without item and splitted (for use with
 * uo::SecureTradeWin)
 *                         place_item_in_secure_trade_container splitted (for use with
 * uo::MoveItemToSecureTradeWin)
 * - 2005/06/01 Shinigami: return_traded_items - added realm support
 * - 2005/11/28 MuadDib:   Added 2 more send_trade_statuses() for freshing of chr and droped_on
 *                         in sending of secure trade windows. This was per packet checks on
 *                         OSI, and works in newer clients. Unable to test in 2.0.0
 * - 2009/07/20 MuadDib:   Slot checks added where can_add() is called.
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/08/06 MuadDib:   Added gotten_by code for items.
 * - 2009/08/09 MuadDib:   Refactor of Packet 0x25 for naming convention
 * - 2009/08/16 MuadDib:   find_giveitem_container(), removed passert, made it return nullptr to
 * reject move instead of a crash. Added slot support to find_giveitem_container()
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 * - 2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
 * - 2009/11/19 Turley:    removed sysmsg after CanInsert call (let scripter handle it) - Tomi
 *
 * @note FIXME: Does STW use slots with KR or newest 2d? If so, we must do slot checks there too.
 */


#include <cstdio>
#include <string>

#include "../bscript/berror.h"
#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/random.h"
#include "../clib/rawtypes.h"
#include "../plib/systemstate.h"
#include "containr.h"
#include "eventid.h"
#include "fnsearch.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "los.h"
#include "mobile/charactr.h"
#include "mobile/npc.h"
#include "multi/multi.h"
#include "network/client.h"
#include "network/clientio.h"
#include "network/packetdefs.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktboth.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "objtype.h"
#include "polcfg.h"
#include "realms/realm.h"
#include "reftypes.h"
#include "statmsg.h"
#include "storage.h"
#include "syshook.h"
#include "systems/suspiciousacts.h"
#include "ufunc.h"
#include "uobject.h"
#include "uoscrobj.h"
#include "uworld.h"

namespace Pol
{
namespace Core
{
void send_trade_statuses( Mobile::Character* chr );

bool place_item_in_container( Network::Client* client, Items::Item* item, UContainer* cont, u16 x,
                              u16 y, u8 slotIndex )
{
  ItemRef itemref( item );
  if ( ( cont->serial == item->serial ) || is_a_parent( cont, item->serial ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }

  if ( !cont->can_add( *item ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    send_sysmessage( client, "That item is too heavy for the container or the container is full." );
    return false;
  }
  if ( !cont->can_insert_add_item( client->chr, UContainer::MT_PLAYER, item ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }

  if ( item->orphan() )
  {
    return true;
  }

  if ( !cont->can_add_to_slot( slotIndex ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    send_sysmessage( client, "The container has no free slots available!" );
    return false;
  }
  if ( !item->slot_index( slotIndex ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    send_sysmessage( client, "The container has no free slots available!" );
    return false;
  }

  item->set_dirty();

  client->pause();
  send_remove_object_to_inrange( item );

  item->x = x;
  item->y = y;

  cont->add( item );
  cont->restart_decay_timer();
  if ( !item->orphan() )
  {
    send_put_in_container_to_inrange( item );

    cont->on_insert_add_item( client->chr, UContainer::MT_PLAYER, item );
  }

  client->restart();
  return true;
}

bool do_place_item_in_secure_trade_container( Network::Client* client, Items::Item* item,
                                              UContainer* cont, Mobile::Character* dropon, u16 x,
                                              u16 y, u8 move_type );
bool place_item_in_secure_trade_container( Network::Client* client, Items::Item* item, u16 x,
                                           u16 y )
{
  UContainer* cont = client->chr->trade_container();
  Mobile::Character* dropon = client->chr->trading_with.get();
  if ( dropon == nullptr || dropon->client == nullptr )
  {
    send_sysmessage( client, "Unable to complete trade" );
    return false;
  }
  if ( gamestate.system_hooks.can_trade )
  {
    if ( !gamestate.system_hooks.can_trade->call( new Module::ECharacterRefObjImp( client->chr ),
                                                  new Module::ECharacterRefObjImp( dropon ),
                                                  new Module::EItemRefObjImp( item ) ) )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
      return false;
    }
  }
  if ( !cont->can_add( *item ) )
  {
    send_sysmessage( client, "That's too heavy to trade." );
    return false;
  }
  if ( !cont->can_insert_add_item( client->chr, UContainer::MT_PLAYER, item ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }

  // FIXME : Add Grid Index Default Location Checks here.
  // Remember, if index fails, move to the ground. That is, IF secure trade uses
  // grid index.

  return do_place_item_in_secure_trade_container( client, item, cont, dropon, x, y, 0 );
}

Bscript::BObjectImp* place_item_in_secure_trade_container( Network::Client* client,
                                                           Items::Item* item )
{
  UContainer* cont = client->chr->trade_container();
  Mobile::Character* dropon = client->chr->trading_with.get();
  if ( dropon == nullptr || dropon->client == nullptr )
  {
    return new Bscript::BError( "Unable to complete trade" );
  }
  if ( gamestate.system_hooks.can_trade )
  {
    if ( !gamestate.system_hooks.can_trade->call( new Module::ECharacterRefObjImp( client->chr ),
                                                  new Module::ECharacterRefObjImp( dropon ),
                                                  new Module::EItemRefObjImp( item ) ) )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
      return new Bscript::BError( "Could not insert item into container." );
    }
  }
  if ( !cont->can_add( *item ) )
  {
    return new Bscript::BError( "That's too heavy to trade." );
  }
  if ( !cont->can_insert_add_item( client->chr, UContainer::MT_CORE_MOVED, item ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return new Bscript::BError( "Could not insert item into container." );
  }

  // FIXME : Add Grid Index Default Location Checks here.
  // Remember, if index fails, move to the ground.

  if ( do_place_item_in_secure_trade_container(
           client, item, cont, dropon, 5 + static_cast<u16>( Clib::random_int( 44 ) ),
           5 + static_cast<u16>( Clib::random_int( 44 ) ), 1 ) )
    return new Bscript::BLong( 1 );
  else
    return new Bscript::BError( "Something went wrong with trade window." );
}

bool do_place_item_in_secure_trade_container( Network::Client* client, Items::Item* item,
                                              UContainer* cont, Mobile::Character* dropon, u16 x,
                                              u16 y, u8 move_type )
{
  client->pause();

  client->chr->trade_accepted( false );
  dropon->trade_accepted( false );
  send_trade_statuses( client->chr );

  send_remove_object_to_inrange( item );

  item->x = x;
  item->y = y;
  item->z = 9;

  cont->add( item );

  send_put_in_container( client, item );
  send_put_in_container( dropon->client, item );

  if ( move_type == 1 )
    cont->on_insert_add_item( client->chr, UContainer::MT_CORE_MOVED, item );
  else
    cont->on_insert_add_item( client->chr, UContainer::MT_PLAYER, item );

  send_trade_statuses( client->chr );
  send_trade_statuses( dropon->client->chr );

  client->restart();
  return true;
}

bool add_item_to_stack( Network::Client* client, Items::Item* item, Items::Item* target_item )
{
  // TJ 3/18/3: Only called (so far) from place_item(), so it's only ever from
  // a player; this means that there's no need to worry about passing
  // UContainer::MT_CORE_* constants to the can_insert function (yet). :)

  ItemRef itemref( item );
  if ( ( !target_item->stackable() ) || ( !target_item->can_add_to_self( *item, false ) ) ||
       ( target_item->container &&
         !target_item->container->can_insert_increase_stack(
             client->chr, UContainer::MT_PLAYER, target_item, item->getamount(), item ) ) )
  {
    send_sysmessage( client, "Could not add item to stack." );
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );

    return false;
  }

  if ( item->orphan() )
    return false;

  /* At this point, we know:
       the object types match
       the combined stack isn't 'too large'
       We don't know: (FIXME)
       if a container that the target_item is in will overfill from this
       */

  send_remove_object_to_inrange( item );

  u16 amtadded = item->getamount();

  target_item->add_to_self( item );
  update_item_to_inrange( target_item );

  if ( target_item->container )
  {
    target_item->container->on_insert_increase_stack( client->chr, UContainer::MT_PLAYER,
                                                      target_item, amtadded );
    target_item->container->restart_decay_timer();
  }
  return true;
}

bool place_item( Network::Client* client, Items::Item* item, u32 target_serial, u16 x, u16 y,
                 u8 slotIndex )
{
  Items::Item* target_item = find_legal_item( client->chr, target_serial );

  if ( target_item == nullptr && client->chr->is_trading() )
  {
    UContainer* cont = client->chr->trade_container();
    if ( target_serial == cont->serial )
    {
      if ( item->no_drop() )
      {
        send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
        return false;
      }
      return place_item_in_secure_trade_container( client, item, x, y );
    }
  }

  if ( target_item == nullptr )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }

  if ( ( pol_distance( client->chr, target_item ) > 2 ) && !client->chr->can_moveanydist() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_TOO_FAR_AWAY );
    return false;
  }
  if ( !client->chr->realm->has_los( *client->chr, *target_item->toplevel_owner() ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_OUT_OF_SIGHT );
    return false;
  }


  if ( target_item->isa( UOBJ_CLASS::CLASS_ITEM ) )
  {
    if ( item->no_drop() )
    {
      if ( target_item->container == nullptr || !target_item->container->no_drop_exception() )
      {
        send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
        return false;
      }
    }
    return add_item_to_stack( client, item, target_item );
  }
  else if ( target_item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    if ( item->no_drop() && !( static_cast<UContainer*>( target_item )->no_drop_exception() ) )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
      return false;
    }
    return place_item_in_container( client, item, static_cast<UContainer*>( target_item ), x, y,
                                    slotIndex );
  }
  else
  {
    // UNTESTED CLIENT_HOLE?
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );

    return false;
  }
}

bool drop_item_on_ground( Network::Client* client, Items::Item* item, u16 x, u16 y, s8 z )
{
  if ( item->no_drop() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }

  Mobile::Character* chr = client->chr;

  Multi::UMulti* multi;
  short newz;
  if ( !inrangex( chr, x, y, 2 ) && !client->chr->can_moveanydist() )
  {
    POLLOG_ERROR.Format( "Client (Character {}) tried to drop an item out of range.\n" )
        << client->chr->name();
    send_item_move_failure( client, MOVE_ITEM_FAILURE_TOO_FAR_AWAY );
    return false;
  }

  if ( !chr->realm->dropheight( x, y, z, client->chr->z, &newz, &multi ) )
  {
    POLLOG_ERROR.Format(
        "Client (Character {}) tried to drop an item at ({},{},{}), which is a blocked "
        "location.\n" )
        << client->chr->name() << x << y << (int)z;
    return false;
  }

  LosObj tgt( x, y, static_cast<s8>( newz ), chr->realm );
  if ( !chr->realm->has_los( *client->chr, tgt ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_OUT_OF_SIGHT );
    return false;
  }

  item->set_dirty();
  item->restart_decay_timer();
  item->x = x;
  item->y = y;
  item->z = static_cast<s8>( newz );
  if ( item->realm != chr->realm )
  {
    if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      UContainer* cont = static_cast<UContainer*>( item );
      cont->for_each_item( setrealm, (void*)chr->realm );
    }
    setrealm( item, (void*)chr->realm );
  }
  item->container = nullptr;
  item->reset_slot();
  add_item_to_world( item );
  if ( multi != nullptr )
  {
    multi->register_object( item );
  }

  send_item_to_inrange( item );
  return true;
}

UContainer* find_giveitem_container( Items::Item* item_to_add, u8 slotIndex )
{
  StorageArea* area = gamestate.storage.create_area( "GivenItems" );
  passert( area != nullptr );

  for ( unsigned short i = 0; i < 500; ++i )
  {
    std::string name = "Cont" + Clib::tostring( i );
    Items::Item* item = nullptr;
    item = area->find_root_item( name );
    if ( item == nullptr )
    {
      item = Items::Item::create( UOBJ_BACKPACK );
      item->setname( name );
      item->realm = find_realm( std::string( "britannia" ) );
      area->insert_root_item( item );
    }
    // Changed this from a passert to return null.
    if ( !( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) ) )
      return nullptr;
    UContainer* cont = static_cast<UContainer*>( item );
    if ( !cont->can_add_to_slot( slotIndex ) )
    {
      return nullptr;
    }
    if ( !item_to_add->slot_index( slotIndex ) )
    {
      return nullptr;
    }
    if ( cont->can_add( *item_to_add ) )
      return cont;
  }
  return nullptr;
}

void send_trade_container( Network::Client* client, Mobile::Character* whos, UContainer* cont )
{
  auto msg =
      Network::AddItemContainerMsg( cont->serial_ext, cont->graphic, 1 /*amount*/, 0 /*x*/, 0 /*y*/,
                                    cont->slot_index(), whos->serial_ext, cont->color );
  msg.Send( client );
}

bool do_open_trade_window( Network::Client* client, Items::Item* item, Mobile::Character* dropon );
bool open_trade_window( Network::Client* client, Items::Item* item, Mobile::Character* dropon )
{
  if ( !Plib::systemstate.config.enable_secure_trading )
  {
    send_sysmessage( client, "Secure trading is unavailable." );
    return false;
  }

  if ( !settingsManager.ssopt.allow_secure_trading_in_warmode )
  {
    if ( dropon->warmode() )
    {
      send_sysmessage( client, "You cannot trade with someone in war mode." );
      return false;
    }
    if ( client->chr->warmode() )
    {
      send_sysmessage( client, "You cannot trade while in war mode." );
      return false;
    }
  }
  if ( dropon->is_trading() )
  {
    send_sysmessage( client, "That person is already involved in a trade." );
    return false;
  }
  if ( client->chr->is_trading() )
  {
    send_sysmessage( client, "You are already involved in a trade." );
    return false;
  }
  if ( !dropon->client )
  {
    send_sysmessage( client, "That person is already involved in a trade." );
    return false;
  }
  if ( client->chr->dead() || dropon->dead() )
  {
    send_sysmessage( client, "Ghosts cannot trade items." );
    return false;
  }

  return do_open_trade_window( client, item, dropon );
}

Bscript::BObjectImp* open_trade_window( Network::Client* client, Mobile::Character* dropon )
{
  if ( !Plib::systemstate.config.enable_secure_trading )
  {
    return new Bscript::BError( "Secure trading is unavailable." );
  }

  if ( !settingsManager.ssopt.allow_secure_trading_in_warmode )
  {
    if ( dropon->warmode() )
    {
      return new Bscript::BError( "You cannot trade with someone in war mode." );
    }
    if ( client->chr->warmode() )
    {
      return new Bscript::BError( "You cannot trade while in war mode." );
    }
  }
  if ( dropon->is_trading() )
  {
    return new Bscript::BError( "That person is already involved in a trade." );
  }
  if ( client->chr->is_trading() )
  {
    return new Bscript::BError( "You are already involved in a trade." );
  }
  if ( !dropon->client )
  {
    return new Bscript::BError( "That person is already involved in a trade." );
  }
  if ( client->chr->dead() || dropon->dead() )
  {
    return new Bscript::BError( "Ghosts cannot trade items." );
  }

  if ( do_open_trade_window( client, nullptr, dropon ) )
    return new Bscript::BLong( 1 );
  else
    return new Bscript::BError( "Something goes wrong." );
}

bool do_open_trade_window( Network::Client* client, Items::Item* item, Mobile::Character* dropon )
{
  dropon->create_trade_container();
  client->chr->create_trade_container();

  dropon->trading_with.set( client->chr );
  dropon->trade_accepted( false );
  client->chr->trading_with.set( dropon );
  client->chr->trade_accepted( false );

  send_trade_container( client, dropon, dropon->trade_container() );
  send_trade_container( dropon->client, dropon, dropon->trade_container() );
  send_trade_container( client, client->chr, client->chr->trade_container() );
  send_trade_container( dropon->client, client->chr, client->chr->trade_container() );

  Network::PktHelper::PacketOut<Network::PktOut_6F> msg;
  msg->WriteFlipped<u16>( sizeof msg->buffer );
  msg->Write<u8>( PKTBI_6F::ACTION_INIT );
  msg->Write<u32>( dropon->serial_ext );
  msg->Write<u32>( client->chr->trade_container()->serial_ext );
  msg->Write<u32>( dropon->trade_container()->serial_ext );
  msg->Write<u8>( 1u );
  msg->Write( dropon->name().c_str(), 30, false );

  msg.Send( client );

  msg->offset = 4;
  msg->Write<u32>( client->chr->serial_ext );
  msg->Write<u32>( dropon->trade_container()->serial_ext );
  msg->Write<u32>( client->chr->trade_container()->serial_ext );
  msg->offset++;  // u8 havename same as above
  msg->Write( client->chr->name().c_str(), 30, false );
  msg.Send( dropon->client );

  if ( item != nullptr )
    return place_item_in_secure_trade_container( client, item, 20, 20 );
  else
    return true;
}

bool drop_item_on_mobile( Network::Client* client, Items::Item* item, u32 target_serial,
                          u8 slotIndex )
{
  Mobile::Character* dropon = find_character( target_serial );

  if ( dropon == nullptr )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }

  if ( pol_distance( client->chr, dropon ) > 2 && !client->chr->can_moveanydist() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_TOO_FAR_AWAY );
    return false;
  }
  if ( !client->chr->realm->has_los( *client->chr, *dropon ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_OUT_OF_SIGHT );
    return false;
  }

  if ( !dropon->isa( UOBJ_CLASS::CLASS_NPC ) )
  {
    if ( item->no_drop() )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
      return false;
    }
    if ( gamestate.system_hooks.can_trade )
    {
      if ( !gamestate.system_hooks.can_trade->call( new Module::ECharacterRefObjImp( client->chr ),
                                                    new Module::ECharacterRefObjImp( dropon ),
                                                    new Module::EItemRefObjImp( item ) ) )
      {
        send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
        return false;
      }
    }
    bool res = open_trade_window( client, item, dropon );
    if ( !res )
      send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return res;
  }

  Mobile::NPC* npc = static_cast<Mobile::NPC*>( dropon );
  if ( !npc->can_accept_event( EVID_ITEM_GIVEN ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }
  if ( item->no_drop() && !npc->no_drop_exception() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }

  UContainer* cont = find_giveitem_container( item, slotIndex );
  if ( cont == nullptr )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }

  if ( !cont->can_add_to_slot( slotIndex ) || !item->slot_index( slotIndex ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }

  client->pause();

  send_remove_object_to_inrange( item );

  u16 rx, ry;
  cont->get_random_location( &rx, &ry );

  item->set_dirty();
  item->container = cont;
  item->x = rx;
  item->y = ry;

  cont->add_at_random_location( item );

  npc->send_event( new Module::ItemGivenEvent( client->chr, item, npc ) );

  client->restart();

  return true;
}

// target_serial should indicate a character, or a container, but not a pile.
bool drop_item_on_object( Network::Client* client, Items::Item* item, u32 target_serial,
                          u8 slotIndex )
{
  ItemRef itemref( item );
  UContainer* cont = nullptr;
  if ( IsCharacter( target_serial ) )
  {
    if ( target_serial == client->chr->serial )
    {
      cont = client->chr->backpack();
    }
    else
    {
      return drop_item_on_mobile( client, item, target_serial, slotIndex );
    }
  }

  if ( cont == nullptr )
  {
    cont = find_legal_container( client->chr, target_serial );
  }

  if ( cont == nullptr )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }
  if ( item->no_drop() && !cont->no_drop_exception() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }
  if ( pol_distance( client->chr, cont ) > 2 && !client->chr->can_moveanydist() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_TOO_FAR_AWAY );
    return false;
  }
  if ( !client->chr->realm->has_los( *client->chr, *cont->toplevel_owner() ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_OUT_OF_SIGHT );
    return false;
  }

  if ( item->stackable() )
  {
    for ( UContainer::const_iterator itr = cont->begin(); itr != cont->end(); ++itr )
    {
      Items::Item* exitem = GET_ITEM_PTR( itr );
      if ( exitem->can_add_to_self( *item, false ) )
      {
        if ( cont->can_add( *item ) )
        {
          if ( cont->can_insert_increase_stack( client->chr, UContainer::MT_PLAYER, exitem,
                                                item->getamount(), item ) )
          {
            if ( item->orphan() )
            {
              return true;
            }
            u16 amtadded = item->getamount();
            exitem->add_to_self( item );
            update_item_to_inrange( exitem );

            cont->on_insert_increase_stack( client->chr, UContainer::MT_PLAYER, exitem, amtadded );

            return true;
          }
        }
        return false;
      }
    }
  }

  u16 rx, ry;
  cont->get_random_location( &rx, &ry );

  return place_item_in_container( client, item, cont, rx, ry, slotIndex );
}

/* DROP_ITEM messages come in a couple varieties:

    1)  Dropping an item on another object, or a person:
    item_serial: serial number of item to drop
    x: 0xFFFF
    y: 0xFFFF
    z: 0
    target_serial: serial number of item or character to drop on.

    2)  Dropping an item on the ground:
    item_serial: serial number of item to drop
    x,y,z: position
    target_serial: 0xFFFFFFFF

    3)  Placing an item in a container, or in an existing pile:
    item_serial: serial number of item to drop
    x: x-position
    y: y-position
    z: 0
    target_serial: serial number of item or character or pile to drop on.
    */

/*
    Name:       drop_item
    Details:    Original version of packet is supported by this function.
    Access:     public
    Qualifier:
    Parameter:  Client * client
    Parameter:  PKTIN_08_V1 * msg
    */
void drop_item( Network::Client* client, PKTIN_08_V1* msg )
{
  u32 item_serial = cfBEu32( msg->item_serial );
  u16 x = cfBEu16( msg->x );
  u16 y = cfBEu16( msg->y );
  s8 z = msg->z;
  u32 target_serial = cfBEu32( msg->target_serial );

  Items::Item* item = client->chr->gotten_item();
  if ( item == nullptr )
  {
    SuspiciousActs::DropItemButNoneGotten( client, item_serial );
    return;
  }
  if ( item->serial != item_serial )
  {
    SuspiciousActs::DropItemOtherThanGotten( client, item_serial, item->serial );
    item->gotten_by( nullptr ); // TODO: shouldn't we clear_gotten_item() here?
    return;
  }
  item->inuse( false );
  item->gotten_by( nullptr );
  client->chr->gotten_item( nullptr );

  bool res;
  if ( target_serial == 0xFFffFFffLu )
  {
    res = drop_item_on_ground( client, item, x, y, z );
  }
  else if ( x == 0xFFFF )
  {
    res = drop_item_on_object( client, item, target_serial, 0 );
  }
  else
  {
    Multi::UMulti* multi = system_find_multi( target_serial );

    if ( multi != nullptr )
      res = drop_item_on_ground( client, item, ( multi->x + x ), ( multi->y + y ), z );
    else
      res = place_item( client, item, target_serial, x, y, 0 );
  }

  if ( !item->orphan() )
  {
    if ( !res )
    {
      undo_get_item( client->chr, item );
    }
    item->inuse( false );
    item->gotten_by( nullptr );
  }
  send_full_statmsg( client, client->chr );
}


/*
    Name:       drop_item_v2
    Details:    This is used for the version of the packet introduced in clients 6.0.1.7 2D and
   UO:KR+ to support Slots
    Access:     public
    Qualifier:
    Parameter:  Client * client
    Parameter:  PKTIN_08_V2 * msg
    */
void drop_item_v2( Network::Client* client, PKTIN_08_V2* msg )
{
  u32 item_serial = cfBEu32( msg->item_serial );
  u16 x = cfBEu16( msg->x );
  u16 y = cfBEu16( msg->y );
  s8 z = msg->z;
  u8 slotIndex = msg->slotindex;
  u32 target_serial = cfBEu32( msg->target_serial );

  Items::Item* item = client->chr->gotten_item();
  if ( item == nullptr )
  {
    SuspiciousActs::DropItemButNoneGotten( client, item_serial );
    return;
  }
  if ( item->serial != item_serial )
  {
    SuspiciousActs::DropItemOtherThanGotten( client, item_serial, item->serial );
    item->gotten_by( nullptr );
    return;
  }
  item->inuse( false );
  item->gotten_by( nullptr );
  client->chr->gotten_item( nullptr );

  bool res;
  if ( target_serial == 0xFFffFFffLu )
  {
    res = drop_item_on_ground( client, item, x, y, z );
  }
  else if ( x == 0xFFFF )
  {
    res = drop_item_on_object( client, item, target_serial, slotIndex );
  }
  else
  {
    Multi::UMulti* multi = system_find_multi( target_serial );

    if ( multi != nullptr )
      res = drop_item_on_ground( client, item, ( multi->x + x ), ( multi->y + y ), z );
    else
      res = place_item( client, item, target_serial, x, y, 0 );
  }

  if ( !item->orphan() )
  {
    if ( !res )
    {
      undo_get_item( client->chr, item );
    }
    item->inuse( false );
    item->gotten_by( nullptr );
  }

  Network::PktHelper::PacketOut<Network::PktOut_29> drop_msg;
  drop_msg.Send( client );

  send_full_statmsg( client, client->chr );
}

void return_traded_items( Mobile::Character* chr )
{
  UContainer* cont = chr->trade_container();
  UContainer* bp = chr->backpack();
  if ( cont == nullptr || bp == nullptr )
    return;

  UContainer::Contents tmp;
  cont->extract( tmp );
  while ( !tmp.empty() )
  {
    Items::Item* item = ITEM_ELEM_PTR( tmp.back() );
    tmp.pop_back();
    item->container = nullptr;
    item->layer = 0;

    ItemRef itemref( item );
    if ( bp->can_add( *item ) && bp->can_insert_add_item( chr, UContainer::MT_CORE_MOVED, item ) )
    {
      if ( item->orphan() )
      {
        continue;
      }
      u8 newSlot = 1;
      if ( !bp->can_add_to_slot( newSlot ) || !item->slot_index( newSlot ) )
      {
        item->set_dirty();
        item->x = chr->x;
        item->y = chr->y;
        item->z = chr->z;
        add_item_to_world( item );
        register_with_supporting_multi( item );
        move_item( item, item->x, item->y, item->z, nullptr );
        return;
      }
      bp->add_at_random_location( item );
      if ( chr->client )
        send_put_in_container( chr->client, item );
      bp->on_insert_add_item( chr, UContainer::MT_CORE_MOVED, item );
    }
    else
    {
      item->set_dirty();
      item->x = chr->x;
      item->y = chr->y;
      item->realm = chr->realm;
      add_item_to_world( item );
      register_with_supporting_multi( item );
      move_item( item, chr->x, chr->y, chr->z, nullptr );
    }
  }
}


void cancel_trade( Mobile::Character* chr1 )
{
  Mobile::Character* chr2 = chr1->trading_with.get();

  return_traded_items( chr1 );
  chr1->trading_with.clear();

  if ( chr1->client )
  {
    Network::PktHelper::PacketOut<Network::PktOut_6F> msg;
    msg->WriteFlipped<u16>( 17u );  // no name
    msg->Write<u8>( PKTBI_6F::ACTION_CANCEL );
    msg->Write<u32>( chr1->trade_container()->serial_ext );
    msg->offset += 9;  // u32 cont1_serial, cont2_serial, u8 havename
    msg.Send( chr1->client );
    send_full_statmsg( chr1->client, chr1 );
  }

  if ( chr2 )
  {
    return_traded_items( chr2 );
    chr2->trading_with.clear();
    if ( chr2->client )
    {
      Network::PktHelper::PacketOut<Network::PktOut_6F> msg;
      msg->WriteFlipped<u16>( 17u );  // no name
      msg->Write<u8>( PKTBI_6F::ACTION_CANCEL );
      msg->Write<u32>( chr2->trade_container()->serial_ext );
      msg->offset += 9;  // u32 cont1_serial, cont2_serial, u8 havename
      msg.Send( chr2->client );
      send_full_statmsg( chr2->client, chr2 );
    }
  }
}

void send_trade_statuses( Mobile::Character* chr )
{
  unsigned int stat1 = chr->trade_accepted() ? 1 : 0;
  unsigned int stat2 = chr->trading_with->trade_accepted() ? 1 : 0;

  Network::PktHelper::PacketOut<Network::PktOut_6F> msg;
  msg->WriteFlipped<u16>( 17u );  // no name
  msg->Write<u8>( PKTBI_6F::ACTION_STATUS );
  msg->Write<u32>( chr->trade_container()->serial_ext );
  msg->WriteFlipped<u32>( stat1 );
  msg->WriteFlipped<u32>( stat2 );
  msg->offset++;  // u8 havename
  Network::transmit( chr->client, &msg->buffer, msg->offset );
  msg->offset = 4;
  msg->Write<u32>( chr->trading_with->trade_container()->serial_ext );
  msg->WriteFlipped<u32>( stat2 );
  msg->WriteFlipped<u32>( stat1 );
  msg->offset++;
  msg.Send( chr->trading_with->client );
}

void change_trade_status( Mobile::Character* chr, bool set )
{
  chr->trade_accepted( set );
  send_trade_statuses( chr );
  if ( chr->trade_accepted() && chr->trading_with->trade_accepted() )

  {
    UContainer* cont0 = chr->trade_container();
    UContainer* cont1 = chr->trading_with->trade_container();
    if ( cont0->can_swap( *cont1 ) )
    {
      cont0->swap( *cont1 );
      cancel_trade( chr );
    }
    else
    {
      POLLOG_ERROR.Format( "Can't swap trade containers: ic0={},w0={}, ic1={},w1={}\n" )
          << cont0->item_count() << cont0->weight() << cont1->item_count() << cont1->weight();
    }
  }
}

void handle_secure_trade_msg( Network::Client* client, PKTBI_6F* msg )
{
  if ( !client->chr->is_trading() )
    return;
  switch ( msg->action )
  {
  case PKTBI_6F::ACTION_CANCEL:
    INFO_PRINT_TRACE( 5 ) << "Cancel secure trade\n";
    cancel_trade( client->chr );
    break;

  case PKTBI_6F::ACTION_STATUS:
    bool set;
    set = msg->cont1_serial ? true : false;
    if ( set )
    {
      INFO_PRINT_TRACE( 5 ) << "Set secure trade indicator\n";
    }
    else
    {
      INFO_PRINT_TRACE( 5 ) << "Clear secure trade indicator\n";
    }
    change_trade_status( client->chr, set );
    break;

  default:
    INFO_PRINT_TRACE( 5 ) << "Unknown secure trade action: " << (int)msg->action << "\n";
    break;
  }
}

void cancel_all_trades()
{
  for ( auto& client : networkManager.clients )
  {
    if ( client->ready && client->chr )
    {
      Mobile::Character* chr = client->chr;
      if ( chr->is_trading() )
        cancel_trade( chr );
    }
  }
}
}  // namespace Core
}  // namespace Pol
