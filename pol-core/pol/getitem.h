/** @file
 *
 * @par History
 */


#ifndef __GETITEM_H
#define __GETITEM_H
#include "pol/base/position.h"
namespace Pol
{
namespace Network
{
class Client;
}
namespace Mobile
{
class Character;
}
namespace Items
{
class Item;
}
namespace Core
{
class UContainer;
struct PKTIN_07;

enum class GOTTEN_ITEM_TYPE : u8
{
  GOTTEN_ITEM_ON_GROUND,
  GOTTEN_ITEM_EQUIPPED,
  GOTTEN_ITEM_IN_CONTAINER,
  /// Rendered on one of a corpse's layers, as opposed to loose among its contents. The two are
  /// both "in a container" as far as the item is concerned, but only the first goes back onto a
  /// layer when it is put back.
  GOTTEN_ITEM_ON_CORPSE
};

/**
 * Let go of whatever the character is holding on its cursor, without deciding where it goes.
 *
 * The item is left detached, for the caller to re-home or to hand back with GottenItem::undo().
 * Both halves of the link go, and so does the in-use flag: clearing only the item's half leaves
 * the character holding a ticket for an item that no longer thinks it is held.
 */
void release_gotten_item( Mobile::Character* chr );

class GottenItem
{
public:
  GottenItem() = default;

  Items::Item* item() { return _item; };
  void undo( Mobile::Character* chr );
  static void handle( Network::Client* client, PKTIN_07* msg );

  /**
   * Build the return ticket for an item that is about to be picked up, describing where it is
   * now. Must be called before the item is detached: it reads the item's location and slot.
   */
  static GottenItem for_item( Items::Item* item );

  /**
   * True when putting the item into cont would put it back on the corpse layer it came off.
   *
   * There is no "move within a container" in UO -- rearranging a loot window is a get and a drop
   * like any other -- so this is what keeps a corpse dressed while a player tidies it. The layer
   * itself is not recorded: on a corpse it is always the item's own tile layer, which is why a
   * graphic change while the item was on the cursor takes the answer back to false.
   */
  bool came_off_corpse_layer( const UContainer* cont ) const;

  bool operator==( const GottenItem& o ) const { return _item == o._item; }

private:
  GottenItem( Items::Item* item, const Core::Pos4d& pos );
  Items::Item* _item = nullptr;
  Core::Pos3d _pos = Core::Pos3d( 0, 0, 0 );
  // Use string realm instead of Pos4d, as realm could be deleted when handling
  // the undo
  std::string _realm;
  u32 _owner_serial = 0;
  u8 _slot_index = 0;
  GOTTEN_ITEM_TYPE _source = GOTTEN_ITEM_TYPE::GOTTEN_ITEM_ON_GROUND;
};

}  // namespace Core
}  // namespace Pol
#endif
