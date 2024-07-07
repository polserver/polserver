/** @file
 *
 * @par History
 * - 2009/08/06 MuadDib:   Added gotten_by code for items.
 * - 2009/08/07 MuadDib:   Altered Layer code in equip_item() to use tile_layer prop instead.
 * Cleaner, less vars, less execution.
 */


#include <cstdio>

#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../plib/systemstate.h"
#include "item/item.h"
#include "layers.h"
#include "mobile/charactr.h"
#include "multi/multi.h"
#include "network/client.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "realms/realm.h"
#include "reftypes.h"
#include "systems/suspiciousacts.h"
#include "ufunc.h"

namespace Pol
{
namespace Core
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
    item->gotten_by( nullptr );  // TODO: shouldn't we clear_gotten_item() here?
    return;
  }

  ItemRef itemref( item );

  item->layer = item->tile_layer;
  item->inuse( false );
  item->gotten_by( nullptr );
  client->chr->gotten_item( {} );

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

  if ( !equip_on->equippable( item ) || !item->check_equiptest_scripts( equip_on ) ||
       !item->check_equip_script( equip_on, false ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );
    if ( item->orphan() )
      return;
    info.undo( client->chr );
    return;
  }

  if ( item->orphan() )
    return;

  // Unregister the item if it is on a multi
  if ( item->container == nullptr && !item->has_gotten_by() )
  {
    Multi::UMulti* multi = item->realm()->find_supporting_multi( item->pos3d() );

    if ( multi != nullptr )
      multi->unregister_object( item );
  }

  equip_on->equip( item );
  send_wornitem_to_inrange( equip_on, item );
}
}  // namespace Core
}  // namespace Pol
