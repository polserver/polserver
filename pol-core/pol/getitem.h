/** @file
 *
 * @par History
 */


#ifndef __GETITEM_H
#define __GETITEM_H
#include <string>
#include <variant>

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
  /// Put the item back where it came from, or as close to it as anything will allow. Reads the
  /// ticket and never rewrites it: the ladder of fallbacks runs on locals, so "where did this come
  /// from" answers the same at every step.
  void undo( Mobile::Character* chr ) const;
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
  /// It was lying in the world.
  struct FromGround
  {
    Core::Pos3d pos;
    /// By name rather than a Realms::Realm*: the ticket outlives its origin, and the realm can be
    /// gone by the time the undo runs.
    std::string realm;
  };
  /// It was in an ordinary container, at a cell in the gump and a slot.
  struct FromContainer
  {
    u32 serial = 0;
    Core::Pos2d grid;
    u8 slot = 0;
  };
  /// It was rendered on one of a corpse's layers, as opposed to lying loose among its contents.
  /// Both are "in a container" as far as the item is concerned, but only this one goes back onto a
  /// layer. The layer is not recorded: on a corpse it is always the item's own tile layer.
  struct FromCorpse
  {
    u32 serial = 0;
    Core::Pos2d grid;
    u8 slot = 0;
  };
  /// It was worn. No cell and no slot, because a layer is not a gump.
  struct FromLayer
  {
    u32 serial = 0;
  };

  /// Where the item came from, and the only thing this ticket is. One alternative per home, each
  /// carrying exactly what putting the item back there needs -- so a cell can never be read as a
  /// world position, which is what a shared position field allowed for years.
  ///
  /// Deliberately not an Items::Location, though it mirrors one: this outlives its origin, so it
  /// names owners by serial and re-resolves them, where Location holds raw pointers.
  using Origin = std::variant<FromGround, FromContainer, FromCorpse, FromLayer>;

  GottenItem( Items::Item* item, Origin origin );

  Items::Item* _item = nullptr;
  Origin _origin;
};

}  // namespace Core
}  // namespace Pol
#endif
