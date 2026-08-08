/** @file
 *
 * @par History
 * - 2009/08/06 MuadDib:   Added gotten_by code for items.
 * - 2009/08/07 MuadDib:   Altered Layer code in equip_item() to use tile_layer prop instead.
 * Cleaner, less vars, less execution.
 */


#include <cstdio>

#include "clib/clib_endian.h"
#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "plib/systemstate.h"
#include "pol/getitem.h"
#include "pol/item/item.h"
#include "pol/layers.h"
#include "pol/mobile/charactr.h"
#include "pol/network/client.h"
#include "pol/network/pktdef.h"
#include "pol/network/pktin.h"
#include "pol/reftypes.h"
#include "pol/systems/suspiciousacts.h"
#include "pol/ufunc.h"


namespace Pol::Core
{
void equip_item( Network::Client* client, PKTIN_13* msg )
{
  u32 serial = cfBEu32( msg->serial );
  u8 layer = msg->layer;
  u32 equip_on_serial = cfBEu32( msg->equipped_on );

  if ( ( layer > HIGHEST_LAYER ) || ( layer == 0 ) || client->chr->dead() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );
    return;
  }

  auto info = client->chr->gotten_item();
  Items::Item* item = info.item();

  if ( item == nullptr )
  {
    SuspiciousActs::EquipItemButNoneGotten( client, serial );
    send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );  // 5
    return;
  }

  if ( item->serial != serial )
  {
    SuspiciousActs::EquipItemOtherThanGotten( client, serial, item->serial );
    send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );  // 5
    // The equip names an item other than the one being held, so the held one has no layer to go
    // to: hand it back to where it came from rather than leaving it detached from everything.
    client->chr->clear_gotten_item();
    return;
  }

  ItemRef itemref( item );

  // The layer the item ends up on is decided by the relocate at the bottom, so letting go of the
  // cursor here leaves nothing half-applied for a check below to undo.
  release_gotten_item( client->chr );

  Mobile::Character* equip_on = nullptr;
  if ( equip_on_serial == client->chr->serial )
  {
    equip_on = client->chr;
  }
  else
  {
    equip_on = find_character( equip_on_serial );
    if ( equip_on == nullptr || !client->chr->can_clothe( equip_on ) )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );

      info.undo( client->chr );
      return;
    }
  }

  if ( equip_on->layer_is_equipped( item->tile_layer ) )
  {
    // it appears the client already checks for this, so this code hasn't been exercised.
    // we'll assume client mouse holds on to object
    // 3D Client doesn't check for this!
    send_item_move_failure( client, MOVE_ITEM_FAILURE_ALREADY_WORN );

    info.undo( client->chr );  // added 11/01/03 for 3d client
    return;
  }

  if ( !equip_on->strong_enough_to_equip( item ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );
    // the client now puts the item back where it was before.

    // return the item to wherever it was. (?)
    info.undo( client->chr );
    if ( client->chr == equip_on )
      send_sysmessage( client, "You are not strong enough to use that." );
    else
      send_sysmessage( client, "Insufficient strength to equip that." );
    return;
  }

  if ( !equip_on->equippable( item ) || !item->check_equiptest_scripts( equip_on, client->chr ) ||
       !item->check_equip_script( equip_on, client->chr, false ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );
    if ( item->orphan() )
      return;
    info.undo( client->chr );
    return;
  }

  if ( item->orphan() )
    return;

  // Not equip_on->equip() directly: the equiptest and equip scripts above can have re-homed the
  // item, and relocate detaches it from wherever it actually is before putting it on the layer —
  // including unregistering it from a multi, if a script put it back on the ground.
  if ( !Items::relocate( *item, Items::Equipped{ equip_on, item->tile_layer } ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );
    info.undo( client->chr );
    return;
  }
  send_wornitem_to_inrange( equip_on, item );
}
}  // namespace Pol::Core
