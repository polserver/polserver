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

/// Fresh from create(), or staged during world load. Belongs to no registry.
struct Detached
{
  bool operator==( const Detached& ) const { return true; }
};

/// On the ground: in a realm zone and in the realm's toplevel item list.
struct InWorld
{
  bool operator==( const InWorld& ) const { return true; }
};

/// Inside an ordinary container, including trade windows and the hidden GivenItems containers.
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
struct Equipped
{
  Core::WornItemsContainer* worn;
  u8 layer;

  bool operator==( const Equipped& other ) const
  {
    return worn == other.worn && layer == other.layer;
  }
};

/// On a corpse. Distinct from Equipped: a corpse holds its contents densely and renders the
/// equippable ones on layers.
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
struct OnCursor
{
  Mobile::Character* holder;

  bool operator==( const OnCursor& other ) const { return holder == other.holder; }
};

/// Root item of a storage area. The key is needed as well as the area: StorageArea keys its map
/// by the item's name as captured at insert time, so the area alone cannot find the item again.
struct InStorage
{
  Core::StorageArea* area;
  boost_utils::object_name_flystring key;

  InStorage( Core::StorageArea* area_, const std::string& key_ ) : area( area_ ), key( key_ ) {}

  bool operator==( const InStorage& other ) const { return area == other.area && key == other.key; }
};

/// Intrinsic equipment (the shared wrestling weapon and friends): a real serial, in no container
/// and no zone, handed to every character at once. Entry-only.
struct Intrinsic
{
  bool operator==( const Intrinsic& ) const { return true; }
};

/// The item became a bit in a spellbook's bitwise_contents. Terminal.
struct Absorbed
{
  bool operator==( const Absorbed& ) const { return true; }
};

/// destroy() has been called; the objecthash still holds it until Reap(). Terminal.
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
  Location() : alt_( Detached{} ) {}

  // Implicit by design: relocate( item, InWorld{} ) should read as it does.
  Location( Detached alt ) : alt_( alt ) {}
  Location( InWorld alt ) : alt_( alt ) {}
  Location( InContainer alt ) : alt_( alt ) {}
  Location( Equipped alt ) : alt_( alt ) {}
  Location( OnCorpse alt ) : alt_( alt ) {}
  Location( OnCursor alt ) : alt_( alt ) {}
  Location( InStorage alt ) : alt_( std::move( alt ) ) {}
  Location( Intrinsic alt ) : alt_( alt ) {}
  Location( Absorbed alt ) : alt_( alt ) {}
  Location( Destroyed alt ) : alt_( alt ) {}

  template <typename T>
  bool holds() const
  {
    return std::holds_alternative<T>( alt_ );
  }

  template <typename T>
  const T& get() const
  {
    passert_always( holds<T>() );
    return std::get<T>( alt_ );
  }

  /// nullptr if the location is not a T. Prefer this over holds() + get() in branchy code.
  template <typename T>
  const T* get_if() const
  {
    return std::get_if<T>( &alt_ );
  }

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

private:
  std::variant<Detached, InWorld, InContainer, Equipped, OnCorpse, OnCursor, InStorage, Intrinsic,
               Absorbed, Destroyed>
      alt_;
};

/**
 * Move an item to a new home, maintaining every registry that home implies.
 *
 * The target is validated completely before anything is touched, so a rejected move leaves the
 * item bit-for-bit as it was; there is no partially-applied state. Returns false on rejection,
 * having logged the item, the current location and the target.
 *
 * @warning Requires PolLock. Must never be called from the world-save path: the save serializes
 *          across gamestate.task_thread_pool while the caller holds the lock.
 */
[[nodiscard]] bool relocate( Item& item, Location to );
}  // namespace Pol::Items

#endif
