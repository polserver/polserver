/** @file
 *
 * Where an item is.
 *
 * Historically an item's location was the implied intersection of several independent fields
 * (`Item::container`, membership in a zone vector, `Item::layer`, `gotten_by`, ...), with nothing
 * enforcing which combinations were legal. `Location` names each legal home exactly once, and
 * `relocate()` is the only function that moves an item between them.
 */

#ifndef ITEM_LOCATION_H
#define ITEM_LOCATION_H

#include <string>
#include <variant>

#include "clib/boostutils.h"
#include "clib/passert.h"
#include "clib/rawtypes.h"
#include "pol/base/position.h"

namespace Pol
{
namespace Core
{
class StorageArea;
class UContainer;
class UCorpse;
class WornItemsContainer;
}  // namespace Core
namespace Mobile
{
class Character;
}  // namespace Mobile
}  // namespace Pol

namespace Pol::Items
{
class Item;

/// Constructed but not finished being set up: no serial yet, so not in the objecthash either.
///
/// Where every item begins, and never a destination: construction does not repeat, and location()
/// answers Preparing ahead of the orphan() test, so an item sent back here would stop being able to
/// report Destroyed. Left for any home once the item has a serial.
///
/// Distinct from Destroyed, which is derived from serial == 0 and would otherwise swallow this
/// state. Item::create() assigns a serial and leaves Detached before it returns, so the only
/// lasting residents are the objects built with a bare new: intrinsic equipment, until startup
/// finishes allocating its serials, and a character's worn-items container, which never has a
/// serial of its own and never joins a registry.
struct Preparing
{
  bool operator==( const Preparing& ) const { return true; }
};

/// Has a serial and belongs to no registry: fresh from create(), or staged during world load.
///
/// Reached from any home by detach(), and briefly from inside relocate() itself, which records it
/// between unlinking and relinking so that anything running in between sees the truth. Left for any
/// home, unconditionally. The one alternative with no rules in either direction.
struct Detached
{
  bool operator==( const Detached& ) const { return true; }
};

/// On the ground: in a realm zone and in the realm's toplevel item list.
///
/// Entered from any other home, given a realm -- but never from InWorld. It is the one alternative
/// carrying no data of its own, so a world-to-world move compares equal to itself and would move
/// nothing; that question belongs to place_at(). Left for any home.
struct InWorld
{
  bool operator==( const InWorld& ) const { return true; }
};

/// Inside an ordinary container, including trade windows and the hidden GivenItems containers.
///
/// Entered when the container is live and is neither the item itself nor somewhere inside it, and
/// the slot is within its range. Capacity is deliberately not checked -- callers have always tested
/// can_add() themselves. Left for any home.
struct InContainer
{
  Core::UContainer* cont;
  Core::Pos2d grid;  ///< position within the container gump
  u8 slot;

  bool operator==( const InContainer& other ) const
  {
    return cont == other.cont && grid == other.grid && slot == other.slot;
  }
};

/// Worn by a character. The layer is the location, not a field on the item.
///
/// Entered on the item's own tile_layer, and only if the character's equippable() agrees -- the
/// same predicate the equip paths use, so relocate cannot drift from them. Left for any home, which
/// is what unequips it.
///
/// Names the character rather than its worn-items container: what may be equipped depends on the
/// character's strength and current weapon as much as on the layer array, and every caller has a
/// character to hand.
struct Equipped
{
  Mobile::Character* chr;
  u8 layer;

  bool operator==( const Equipped& other ) const
  {
    return chr == other.chr && layer == other.layer;
  }
};

/// On a corpse. Distinct from Equipped: a corpse holds its contents densely and renders the
/// equippable ones on layers.
///
/// Entered on a live corpse, on a free equippable layer that is the item's own tile_layer. There is
/// no layer-less form -- loose corpse contents are InContainer like anything else -- so being here
/// is exactly what "the corpse renders this" means. Left for any home, which stops it being drawn.
struct OnCorpse
{
  Core::UCorpse* corpse;
  Core::Pos2d grid;
  u8 slot;
  u8 layer;

  bool operator==( const OnCorpse& other ) const
  {
    return corpse == other.corpse && grid == other.grid && slot == other.slot &&
           layer == other.layer;
  }
};

/// Held on a client's cursor between a get and the matching drop/equip.
///
/// Derived from the gotten_by link rather than stored, so it cannot fall out of step with it.
/// Entered when the holder is not already holding something and the item has a realm to be returned
/// to. Left for any home; the drop paths and undo are what do it.
struct OnCursor
{
  Mobile::Character* holder;

  bool operator==( const OnCursor& other ) const { return holder == other.holder; }
};

/// Root item of a storage area. The key is needed as well as the area: StorageArea keys its map
/// by the item's name as captured at insert time, so the area alone cannot find the item again.
///
/// Entered under a key that is the item's name. Left for any home, but only while the area still
/// files it under that key -- a rename behind the area's back strands the item, and refusing here
/// is what stops a half-applied move.
struct InStorage
{
  Core::StorageArea* area;
  boost_utils::object_name_flystring key;

  InStorage( Core::StorageArea* area_, const std::string& key_ ) : area( area_ ), key( key_ ) {}

  bool operator==( const InStorage& other ) const { return area == other.area && key == other.key; }
};

/// Which of the two slots a piece of intrinsic equipment fills. Deliberately not a layer: an
/// intrinsic weapon is never worn. The layer it reports is derived from this, in one place.
enum class IntrinsicKind : u8
{
  Weapon,
  Shield
};

/// Intrinsic equipment (the shared wrestling weapon and friends): a real serial, in no container
/// and no zone, handed to every character at once.
///
/// Reached only from Preparing, and never left: the item is shared, so moving it would move it out
/// from under every character holding it.
struct Intrinsic
{
  IntrinsicKind kind;

  bool operator==( const Intrinsic& other ) const { return kind == other.kind; }
};

/// destroy() has been called; the objecthash still holds it until Reap().
///
/// Derived from serial == 0 rather than stored. Never a relocate target -- destruction is
/// destroy(), which unlinks and ends the item, not a move to somewhere -- and never left.
struct Destroyed
{
  bool operator==( const Destroyed& ) const { return true; }
};

/**
 * Opaque sum of the alternatives above.
 *
 * Callers see only holds<T>(), get<T>() and relocate(); nothing outside this header may touch the
 * variant. That is what allows the representation to be swapped for a tagged union later without
 * call-site churn, should the per-item cost matter.
 */
class Location
{
public:
  Location() : alt_( Preparing{} ) {}

  // Implicit by design: relocate( item, InWorld{} ) should read as it does.
  Location( Preparing alt ) : alt_( alt ) {}
  Location( Detached alt ) : alt_( alt ) {}
  Location( InWorld alt ) : alt_( alt ) {}
  Location( InContainer alt ) : alt_( alt ) {}
  Location( Equipped alt ) : alt_( alt ) {}
  Location( OnCorpse alt ) : alt_( alt ) {}
  Location( OnCursor alt ) : alt_( alt ) {}
  Location( InStorage alt ) : alt_( std::move( alt ) ) {}
  Location( Intrinsic alt ) : alt_( alt ) {}
  Location( Destroyed alt ) : alt_( alt ) {}

  template <typename T>
  bool holds() const
  {
    return std::holds_alternative<T>( alt_ );
  }

  // Both of these hand out a reference into the Location, and Item::location() returns one by
  // value, so `item->location().get<T>()` would leave the caller holding a pointer into a
  // temporary that is already gone by the next statement. Refusing to answer an rvalue turns that
  // into a compile error: name the Location first, then ask it.
  template <typename T>
  const T& get() const&
  {
    passert_always( holds<T>() );
    return std::get<T>( alt_ );
  }
  template <typename T>
  const T& get() const&& = delete;

  /// nullptr if the location is not a T. Prefer this over holds() + get() in branchy code.
  template <typename T>
  const T* get_if() const&
  {
    return std::get_if<T>( &alt_ );
  }
  template <typename T>
  const T* get_if() const&& = delete;

  bool operator==( const Location& other ) const { return alt_ == other.alt_; }
  bool operator!=( const Location& other ) const { return !( *this == other ); }

  /// Human-readable, for assertion messages and the integrity sweep.
  std::string describe() const;

  /// The container half of the location, with today's Item::container semantics: the worn-items
  /// container when equipped, the corpse when on a corpse, nullptr everywhere else.
  Core::UContainer* container() const;

  /// The layer half, with today's Item::layer semantics: zero unless equipped or on a corpse.
  u8 layer() const;

  /// The slot half, with today's Item::slot_index() semantics.
  u8 slot() const;

  /// Where the item sits in the gump of whatever holds it; a default Pos2d for the homes that have
  /// no gump. This is the authoritative answer -- the same coordinates also reach the item's own
  /// position field, because attach() hands this grid to UContainer::add, but that copy exists only
  /// to feed readers that have not been pointed here yet.
  Core::Pos2d grid() const;

private:
  std::variant<Preparing, Detached, InWorld, InContainer, Equipped, OnCorpse, OnCursor, InStorage,
               Intrinsic, Destroyed>
      alt_;
};

/**
 * Take the item out of whatever registry it is currently filed in, leaving it Detached.
 *
 * This is the half of relocate() that has no destination: it is what destruction needs, and what a
 * caller that is about to re-home an item by hand needs. Doing nothing is a valid outcome -- an
 * item that is Preparing, already Detached, or destroyed has no registry to leave -- so calling it
 * twice, or on an item a script has already disposed of, is harmless.
 */
void detach( Item& item );

/**
 * Forget where the item is without touching the registry that holds it.
 *
 * Only for the case where that registry is itself being torn down: a container destroying its
 * contents, a storage area being deleted, the shutdown sweep clearing a realm's zones. There is
 * nothing to unlink from, and unlinking would mean walking a list that is being cleared out from
 * underneath -- or, for worn items, running the unequip bookkeeping of a character that is already
 * half destroyed.
 *
 * Anywhere the owner survives the item, use detach().
 */
void abandon( Item& item );

/**
 * Move an item to a new home, maintaining every registry that home implies.
 *
 * The target is validated completely before anything is touched, so a rejected move leaves the
 * item bit-for-bit as it was; there is no partially-applied state. Returns false on rejection,
 * having logged the item, the current location and the target.
 *
 * Entering InWorld also restarts the item's decay timer, which is an effect of the state and not
 * of the caller: Decay::step walks realm zones, so an item decays because it is in one and for no
 * other reason. Telling clients about the move is *not* folded in — see relocate()'s definition.
 *
 * @warning Requires PolLock. Must never be called from the world-save path: the save serializes
 *          across gamestate.task_thread_pool while the caller holds the lock.
 */
[[nodiscard]] bool relocate( Item& item, Location to );

/**
 * The world loader's way in: the same move, without the effects that only make sense on a running
 * shard.
 *
 * Today that means the decay timer, which the save already carries per item — restarting it here
 * would quietly reset decay across the whole world on every restart.
 */
[[nodiscard]] bool relocate_loaded( Item& item, Location to );

/**
 * Put the item at a world position: relocate()'s peer, and the other half of the same question.
 * relocate() says which home holds the item; this says where it is standing once that home is the
 * world.
 *
 * It exists because InWorld is the one alternative that does not carry its own coordinates -- they
 * live on the item -- so `setposition()` followed by `relocate( InWorld{} )` is not a move at all:
 * the target compares equal to the current location, relocate has nothing to do, and the item ends
 * up at the new coordinates still listed in the zone for the old ones. Every other alternative
 * carries the data that makes the same operation work.
 *
 * Handles both cases: an item that is not in the world yet enters it, and an item already in the
 * world moves. Along the way it maintains the two spatial indexes that are keyed on position -- the
 * realm zone and the multi the item is standing on -- and pushes a realm change down to a
 * container's immediate contents.
 *
 * Deliberately does *not* restart the decay timer or tell any client. Doors and boats move items
 * without either; see move_item() for the version that does both.
 *
 * @returns false only if the item is in a state that cannot move at all -- destroyed, intrinsic, or
 *          filed in a storage area under a key that no longer names it. The destination itself can
 *          only be refused for want of a realm, so callers do not need the recovery paths that a
 *          relocate() into a container needs. On refusal the item is untouched, position included.
 */
[[nodiscard]] bool place_at( Item& item, const Core::Pos4d& newpos );

/**
 * Claim a slot in a container and move the item into it.
 *
 * Three steps that have to happen in this order, because can_add_to_slot() writes the slot it
 * settled on back through its argument and the item has to be told before the move. Callers spelled
 * that out one line at a time, and got the failure path subtly wrong doing it: claiming the slot
 * mutates the item, relocate() can still refuse afterwards, and nothing put the old slot back. This
 * does, so a rejected insert really does leave the item as it was -- which is what relocate()
 * promises and what its callers could not deliver on their own.
 *
 * @param slot_hint where to start looking, updated to the slot actually taken. Loops filling one
 *        container should carry it, rather than rescanning from the first slot each time.
 *
 * Deliberately not folded in, at each caller instead:
 *  - container capacity. relocate() does not check it either, callers have always tested can_add()
 *    themselves, and moving it here would turn inserts that succeed today into failures.
 *  - the CanInsert / OnInsert scripts, which can destroy either object and whose placement around
 *    the move differs per caller.
 *  - telling clients. Five different broadcasts are used across these sites and they are not
 *    interchangeable.
 */
[[nodiscard]] bool move_into( Item& item, Core::UContainer& cont, const Core::Pos2d& grid,
                              u8& slot_hint );
[[nodiscard]] bool move_into( Item& item, Core::UContainer& cont, const Core::Pos2d& grid );
/// Anywhere in the container's gump, for callers whose intent is only "inside this one".
[[nodiscard]] bool move_into( Item& item, Core::UContainer& cont, u8& slot_hint );
[[nodiscard]] bool move_into( Item& item, Core::UContainer& cont );
}  // namespace Pol::Items

#endif
