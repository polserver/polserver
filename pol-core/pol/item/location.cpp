#include "pol/item/location.h"

#include <string>
#include <utility>

#include "clib/logfacility.h"
#include "pol/containr.h"
#include "pol/getitem.h"
#include "pol/globals/settings.h"
#include "pol/item/item.h"
#include "pol/layers.h"
#include "pol/mobile/charactr.h"
#include "pol/mobile/corpse.h"
#include "pol/mobile/wornitems.h"
#include "pol/multi/house.h"
#include "pol/multi/multi.h"
#include "pol/realms/realm.h"
#include "pol/storage.h"
#include "pol/ufunc.h"
#include "pol/uworld.h"

namespace Pol::Items
{
std::string Location::describe() const
{
  if ( holds<Preparing>() )
    return "being prepared";
  if ( holds<Detached>() )
    return "detached";
  if ( holds<InWorld>() )
    return "in the world";
  if ( const auto* alt = get_if<InContainer>() )
    return fmt::format( "in container {:#x}", alt->cont != nullptr ? alt->cont->serial : 0 );
  if ( const auto* alt = get_if<Equipped>() )
    return fmt::format( "equipped on layer {} of {:#x}", alt->layer,
                        alt->chr != nullptr ? alt->chr->serial : 0 );
  if ( const auto* alt = get_if<OnCorpse>() )
    return fmt::format( "on corpse {:#x}, layer {}",
                        alt->corpse != nullptr ? alt->corpse->serial : 0, alt->layer );
  if ( const auto* alt = get_if<OnCursor>() )
    return fmt::format( "on the cursor of {:#x}",
                        alt->holder != nullptr ? alt->holder->serial : 0 );
  if ( const auto* alt = get_if<InStorage>() )
    return fmt::format( "storage root \"{}\"", alt->key.get() );
  if ( const auto* alt = get_if<Intrinsic>() )
    return fmt::format( "intrinsic {}", alt->kind == IntrinsicKind::Weapon ? "weapon" : "shield" );
  return "destroyed";
}

Core::UContainer* Location::container() const
{
  if ( const auto* alt = get_if<InContainer>() )
    return alt->cont;
  if ( const auto* alt = get_if<Equipped>() )
    return alt->chr != nullptr ? alt->chr->worn_items() : nullptr;
  if ( const auto* alt = get_if<OnCorpse>() )
    return alt->corpse;
  return nullptr;
}

u8 Location::layer() const
{
  if ( const auto* alt = get_if<Equipped>() )
    return alt->layer;
  if ( const auto* alt = get_if<OnCorpse>() )
    return alt->layer;
  // Intrinsic equipment is never worn, but scripts read the slot it stands for through
  // chr.weapon.layer / chr.shield.layer, so the one mapping lives here.
  if ( const auto* alt = get_if<Intrinsic>() )
    return static_cast<u8>( alt->kind == IntrinsicKind::Weapon ? Core::LAYER_HAND1
                                                               : Core::LAYER_HAND2 );
  return 0;
}

u8 Location::slot() const
{
  if ( const auto* alt = get_if<InContainer>() )
    return alt->slot;
  if ( const auto* alt = get_if<OnCorpse>() )
    return alt->slot;
  return 0;
}

Location Item::location() const
{
  // An item that has never been given a serial is not destroyed, it is unfinished -- and the
  // orphan() test below cannot tell those apart. Preparing is stored, so it can.
  if ( loc_.holds<Preparing>() )
    return loc_;
  // Both of these are derived rather than stored, so they cannot fall out of step with the rest
  // of the core: destruction is serial == 0, which is what Reap() collects on, and the cursor is
  // the gotten_by link.
  if ( orphan() )
    return Destroyed{};
  if ( has_gotten_by() )
    return OnCursor{ gotten_by() };
  return loc_;
}

void Item::set_location( Location loc )
{
  loc_ = std::move( loc );
}

Core::UContainer* Item::container() const
{
  return location().container();
}

u8 Item::slot_index() const
{
  return location().slot();
}

Mobile::Character* Item::wearer() const
{
  const Location loc = location();
  if ( const auto* equipped = loc.get_if<Equipped>() )
    return equipped->chr;
  return nullptr;
}

void Item::refresh_wearer_ar() const
{
  if ( Mobile::Character* chr = wearer(); chr != nullptr )
    chr->refresh_ar();
}

namespace
{
bool reject( const Item& item, const Location& from, const Location& to, const char* why )
{
  POLLOG_ERRORLN( "relocate: refused to move item {:#x} ({}) to {}: {}", item.serial,
                  from.describe(), to.describe(), why );
  return false;
}

/**
 * Everything that can make the move illegal, checked before anything is touched.
 *
 * Container capacity is deliberately not checked here: callers have always been required to test
 * can_add() themselves, and moving that decision would turn currently-succeeding inserts into
 * failures.
 */
bool validate( const Item& item, const Location& from, const Location& to )
{
  if ( from.holds<Destroyed>() )
    return reject( item, from, to, "the item is destroyed" );
  if ( from.holds<Intrinsic>() )
    return reject( item, from, to, "intrinsic equipment is shared and cannot be moved" );

  if ( const auto* in_storage = from.get_if<InStorage>() )
  {
    if ( in_storage->area->find_root_item( in_storage->key.get() ) != &item )
      return reject( item, from, to, "the storage area no longer files it under that key" );
  }

  // Preparing means "not finished being constructed", which nothing can go back to being -- and
  // Item::location() answers it ahead of the orphan() test, so an item moved here would keep
  // reporting Preparing after being destroyed, escaping the derivation that makes Destroyed
  // terminal.
  if ( to.holds<Preparing>() )
    return reject( item, from, to, "an item cannot go back to being under construction" );

  // Destruction is not a move. It is the one transition with no destination -- Item::destroy()
  // leaves every registry and then ends the item -- and Destroyed is derived from orphan() rather
  // than stored, so there would be nothing here to write anyway.
  if ( to.holds<Destroyed>() )
    return reject( item, from, to, "an item is destroyed rather than moved there" );

  if ( to.holds<Intrinsic>() && !from.holds<Preparing>() )
    return reject( item, from, to, "only a freshly built item can become intrinsic equipment" );

  if ( const auto* on_cursor = to.get_if<OnCursor>() )
  {
    if ( on_cursor->holder == nullptr )
      return reject( item, from, to, "null cursor holder" );
    if ( on_cursor->holder->has_gotten_item() )
      return reject( item, from, to, "that character is already holding something" );
    // The return ticket records a realm by name, so an item that has never had one cannot
    // describe where it should go back to.
    if ( item.realm() == nullptr )
      return reject( item, from, to, "the item has no realm to return to" );
  }

  if ( to.holds<InWorld>() && item.realm() == nullptr )
    return reject( item, from, to, "the item has no realm" );

  if ( const auto* in_cont = to.get_if<InContainer>() )
  {
    if ( in_cont->cont == nullptr )
      return reject( item, from, to, "null container" );
    if ( in_cont->cont->orphan() )
      return reject( item, from, to, "the container is destroyed" );
    if ( in_cont->cont->serial == item.serial )
      return reject( item, from, to, "an item cannot contain itself" );
    if ( Core::is_a_parent( in_cont->cont, item.serial ) )
      return reject( item, from, to, "that would put the item inside itself" );
    if ( Core::settingsManager.ssopt.use_slot_index && in_cont->slot > in_cont->cont->max_slots() )
      return reject( item, from, to, "slot out of range" );
  }
  else if ( const auto* equipped = to.get_if<Equipped>() )
  {
    if ( equipped->chr == nullptr )
      return reject( item, from, to, "null character" );
    if ( equipped->layer != item.tile_layer )
      return reject( item, from, to, "the layer does not match the item's tile layer" );
    // Deliberately the same predicate the equip paths already use, rather than a hand-rolled
    // layer test: it covers the occupied layer, the strength requirement, the two-handed weapon
    // rules and the boat-mount exception to valid_equip_layer in one place, so relocate cannot
    // drift from it.
    if ( !equipped->chr->equippable( &item ) )
      return reject( item, from, to, "the character cannot equip that" );
  }
  else if ( const auto* on_corpse = to.get_if<OnCorpse>() )
  {
    if ( on_corpse->corpse == nullptr )
      return reject( item, from, to, "null corpse" );
    if ( on_corpse->corpse->orphan() )
      return reject( item, from, to, "the corpse is destroyed" );
    // OnCorpse means "the corpse renders this on a layer", so there is no layer-less form of it --
    // loose corpse contents are InContainer like any other contents.
    if ( !valid_equip_layer( on_corpse->layer ) )
      return reject( item, from, to, "not an equippable layer" );
    if ( on_corpse->layer != item.tile_layer )
      return reject( item, from, to, "the layer does not match the item's tile layer" );
    const Items::Item* occupant = on_corpse->corpse->layer_view()[on_corpse->layer];
    if ( occupant != nullptr && occupant->serial != item.serial )
      return reject( item, from, to, "the layer is already occupied" );
    if ( Core::settingsManager.ssopt.use_slot_index &&
         on_corpse->slot > on_corpse->corpse->max_slots() )
      return reject( item, from, to, "slot out of range" );
  }
  else if ( const auto* in_storage = to.get_if<InStorage>() )
  {
    if ( in_storage->area == nullptr )
      return reject( item, from, to, "null storage area" );
    if ( in_storage->key.get() != item.name() )
      return reject( item, from, to, "the storage key must be the item's name" );
  }

  return true;
}

void detach( Item& item, const Location& from )
{
  if ( const auto* in_cont = from.get_if<InContainer>() )
    in_cont->cont->remove( &item );
  else if ( const auto* equipped = from.get_if<Equipped>() )
    equipped->chr->unequip( &item );
  else if ( const auto* on_corpse = from.get_if<OnCorpse>() )
    on_corpse->corpse->Core::UContainer::remove( &item );
  else if ( const auto* in_storage = from.get_if<InStorage>() )
    in_storage->area->remove_root_item( in_storage->key.get(), &item );
  else if ( const auto* on_cursor = from.get_if<OnCursor>() )
  {
    // Both halves, always. Clearing only the item's half is what leaves a character holding a
    // cursor entry for an item that has moved on.
    if ( on_cursor->holder != nullptr )
      on_cursor->holder->gotten_item( {} );
    item.gotten_by( nullptr );
    item.inuse( false );
  }
  else if ( from.holds<InWorld>() )
  {
    // A house component is a world item that the house also lists. Leaving the world ends that:
    // otherwise picking a component up leaves the house holding an entry for an item now sitting
    // in somebody's backpack, which the next house destruction would follow.
    if ( Multi::UHouse* house = item.house(); house != nullptr )
      house->erase_component( &item );
    Core::remove_item_from_world( &item );
  }
}

/// The cursor is the one home whose contents are derived from the previous one, so it is attached
/// by relocate() itself rather than here.
void attach_to_cursor( Item& item, Mobile::Character& holder, const Core::GottenItem& ticket )
{
  holder.gotten_item( ticket );
  item.inuse( true );
  item.gotten_by( &holder );
  item.setposition( Core::Pos4d( 0, 0, 0, item.realm() ) );  // don't let a boat carry it around
}

/// File the item in whichever registry the target names. The alternatives that name none --
/// Detached and Intrinsic -- have nothing to do here; relocate() records the location either way.
void attach( Item& item, const Location& to )
{
  if ( to.holds<InWorld>() )
  {
    Core::add_item_to_world( &item );
    Core::register_with_supporting_multi( &item );
  }
  else if ( const auto* in_cont = to.get_if<InContainer>() )
    in_cont->cont->add( &item, in_cont->grid );
  else if ( const auto* equipped = to.get_if<Equipped>() )
    equipped->chr->equip( &item );
  else if ( const auto* on_corpse = to.get_if<OnCorpse>() )
    on_corpse->corpse->add_rendered_item( &item, on_corpse->grid );
  else if ( const auto* in_storage = to.get_if<InStorage>() )
    in_storage->area->insert_root_item( &item );
}
}  // namespace

void abandon( Item& item )
{
  item.set_location( Detached{} );
}

void detach( Item& item )
{
  detach( item, item.location() );
  abandon( item );
}

/**
 * The loader's entry point is the primitive one, because it wants strictly less: it is relocate()
 * minus the effects that only make sense on a running shard.
 *
 * Deliberately none of those effects is telling a client about the move. That looks like a state
 * effect and is not one: across the callers that put an item in the world, five different things
 * are sent -- send_item_moved, send_item_to_inrange, update_item_to_inrange, send_item_restored,
 * and nothing at all for a multi's own components. They are not interchangeable;
 * send_item_to_inrange alone also carries the invisible and movable flags, a corpse's contents and
 * a revision packet. What a client has to be told depends on why the item moved, not only on where
 * it ended up, so it stays with the caller.
 */
bool relocate_loaded( Item& item, Location to )
{
  // The one place the UseContainerSlots gate lives, now that no field refuses to record a slot.
  // Normalising the target rather than the answer keeps the stored location and every view of it
  // saying the same thing -- a Location that carries a cell the shard does not use would be read
  // back out of get<InContainer>() by validate and by the save.
  if ( !Core::settingsManager.ssopt.use_slot_index )
  {
    if ( const auto* in_cont = to.get_if<InContainer>(); in_cont != nullptr && in_cont->slot != 0 )
      to = InContainer{ in_cont->cont, in_cont->grid, 0 };
    else if ( const auto* on_corpse = to.get_if<OnCorpse>();
              on_corpse != nullptr && on_corpse->slot != 0 )
      to = OnCorpse{ on_corpse->corpse, on_corpse->grid, 0, on_corpse->layer };
  }

  const Location from = item.location();

  // The early return below reads as "the item is already where it is going", and that inference
  // holds for every alternative whose data determines the outcome. InWorld is the exception: it
  // carries no coordinates, so it compares equal to itself no matter how far apart the two
  // positions are, and approving the move would leave the item at the new ones still listed in the
  // zone for the old. Where in the world an item stands is a question this function cannot answer.
  if ( from.holds<InWorld>() && to.holds<InWorld>() )
    return reject( item, from, to, "use place_at() to move an item that is already in the world" );

  if ( from == to )
    return true;

  if ( !validate( item, from, to ) )
    return false;

  // The cursor's return ticket describes where the item is coming from, so it has to be taken
  // before the detach resets the slot index and unlinks the container.
  const auto* to_cursor = to.get_if<OnCursor>();
  Core::GottenItem ticket;
  if ( to_cursor != nullptr )
    ticket = Core::GottenItem::for_item( &item );

  detach( item, from );
  // Between the two halves the item is genuinely nowhere, and it has to say so: attach() asks the
  // item where it is -- registering with a supporting multi tests its container -- and anything a
  // registry runs on the way in can ask too. Four of detach()'s branches already left Detached
  // behind, by way of UContainer::remove and RemoveItemFromLayer; leaving the world and leaving a
  // storage area did not.
  abandon( item );

  if ( to_cursor != nullptr )
  {
    attach_to_cursor( item, *to_cursor->holder, ticket );
    // The location stays Detached. OnCursor is derived from the gotten_by link, so storing it as
    // well would give location() two answers that can disagree; what is true of the registries is
    // that the item has left all of them, which is what the detach above already recorded.
  }
  else
  {
    attach( item, to );
    // The location we validated is the location the item ends up with, rather than whatever the
    // registry we just handed it to would have inferred. Those are not always the same thing: a
    // corpse used to answer OnCorpse for an item its layer list still named, and a spellbook
    // absorbs a scroll without ever chaining to the container it was told to join.
    item.set_location( to );
  }

  return true;
}

bool move_into( Item& item, Core::UContainer& cont, const Core::Pos2d& grid, u8& slot_hint )
{
  return cont.can_add_to_slot( slot_hint ) &&
         relocate( item, InContainer{ &cont, grid, slot_hint } );
}

bool move_into( Item& item, Core::UContainer& cont, const Core::Pos2d& grid )
{
  u8 slot_hint = 1;
  return move_into( item, cont, grid, slot_hint );
}

bool move_into( Item& item, Core::UContainer& cont, u8& slot_hint )
{
  return move_into( item, cont, cont.get_random_location(), slot_hint );
}

bool move_into( Item& item, Core::UContainer& cont )
{
  u8 slot_hint = 1;
  return move_into( item, cont, cont.get_random_location(), slot_hint );
}

bool relocate( Item& item, Location to )
{
  const bool entering_world = to.holds<InWorld>();
  if ( !relocate_loaded( item, std::move( to ) ) )
    return false;

  // An item decays because it is in a realm zone and for no other reason, so entering the world is
  // what starts its clock. Items whose decay was switched off -- a multi's components, say -- are
  // unaffected: restart_decay_timer() does nothing to them.
  if ( entering_world )
    item.restart_decay_timer();

  return true;
}

/**
 * A container's contents borrow their realm from whatever holds them, and nothing re-derives it, so
 * moving the container across realms leaves them behind. Matching the hand-written copies this
 * replaces, it reaches the immediate contents and no further -- nested containers keep the old
 * realm, which is a wider change than this one.
 */
namespace
{
void push_realm_to_contents( Item& item, Realms::Realm* realm )
{
  if ( !item.isa( Core::UOBJ_CLASS::CLASS_CONTAINER ) )
    return;
  auto& cont = static_cast<Core::UContainer&>( item );
  cont.for_each_item( Core::setrealm, static_cast<void*>( realm ) );
}
}  // namespace

bool place_at( Item& item, const Core::Pos4d& newpos )
{
  const Core::Pos4d oldpos = item.pos();

  // Checked here rather than left to validate(), which only sees the entering case, so that both
  // ways in refuse the same thing -- and so the multi lookup below has a realm to ask.
  if ( newpos.realm() == nullptr )
  {
    POLLOG_ERRORLN( "place_at: refused to move item {:#x} ({}) to a position with no realm",
                    item.serial, item.location().describe() );
    return false;
  }

  // Both ways in have to carry it: an item dropped on the ground from a cursor is entering the
  // world, not moving within it, and it can still be changing realm as it lands.
  const bool changing_realm = oldpos.realm() != newpos.realm();

  if ( !item.location().holds<InWorld>() )
  {
    // relocate() promises that a refusal leaves the item exactly as it was, and the position is
    // part of "as it was" -- so it has to be put back by hand, being the one piece of the item's
    // location that relocate does not own.
    item.setposition( newpos );
    if ( !relocate( item, InWorld{} ) )
    {
      item.setposition( oldpos );
      return false;
    }
    if ( changing_realm )
      push_realm_to_contents( item, newpos.realm() );
    return true;
  }

  // Both multis are resolved before anything moves, and the pair is skipped when they match. Not
  // an optimisation: unregister_object() erases from the traveller list and register_object()
  // appends, so an item stepping around one deck would work its way to the end of that list, and
  // the list is both saved and the order the boat is drawn in.
  //
  // This is why the two named helpers are not used here -- they each repeat the lookup, and by
  // this point the answer is known.
  Multi::UMulti* oldmulti = oldpos.realm()->find_supporting_multi( oldpos.xyz() );
  Multi::UMulti* newmulti = newpos.realm()->find_supporting_multi( newpos.xyz() );
  const bool changing_multi = oldmulti != newmulti;

  // Before the position changes: the multi is found from where the item is, so once it has moved
  // there is no way back to the one it joined.
  if ( changing_multi && oldmulti != nullptr )
    oldmulti->unregister_object( &item );

  item.setposition( newpos );
  Core::MoveItemWorldPosition( oldpos, &item );

  if ( changing_realm )
    push_realm_to_contents( item, newpos.realm() );

  if ( changing_multi && newmulti != nullptr )
    newmulti->register_object( &item );

  return true;
}
}  // namespace Pol::Items
