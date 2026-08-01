#include "pol/item/location.h"

#include <string>
#include <utility>

#include "clib/logfacility.h"
#include "pol/containr.h"
#include "pol/getitem.h"
#include "pol/globals/settings.h"
#include "pol/item/item.h"
#include "pol/mobile/charactr.h"
#include "pol/mobile/corpse.h"
#include "pol/mobile/wornitems.h"
#include "pol/multi/house.h"
#include "pol/storage.h"
#include "pol/ufunc.h"
#include "pol/uworld.h"

namespace Pol::Items
{
std::string Location::describe() const
{
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
  if ( holds<Intrinsic>() )
    return "intrinsic equipment";
  if ( holds<Absorbed>() )
    return "absorbed";
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
  if ( from.holds<Absorbed>() )
    return reject( item, from, to, "the item has been absorbed" );
  if ( from.holds<Intrinsic>() )
    return reject( item, from, to, "intrinsic equipment is shared and cannot be moved" );

  if ( const auto* in_storage = from.get_if<InStorage>() )
  {
    if ( in_storage->area->find_root_item( in_storage->key.get() ) != &item )
      return reject( item, from, to, "the storage area no longer files it under that key" );
  }

  if ( to.holds<Intrinsic>() && !from.holds<Detached>() )
    return reject( item, from, to, "intrinsic equipment must be registered before it is placed" );

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
    if ( on_corpse->layer != 0 )
    {
      if ( !valid_equip_layer( on_corpse->layer ) )
        return reject( item, from, to, "not an equippable layer" );
      if ( on_corpse->layer != item.tile_layer )
        return reject( item, from, to, "the layer does not match the item's tile layer" );
      const Core::ItemRef& occupant = on_corpse->corpse->GetItemOnLayer( on_corpse->layer );
      if ( occupant != nullptr && occupant->serial != item.serial )
        return reject( item, from, to, "the layer is already occupied" );
    }
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

/// Returns false for the alternatives that have no registry to join, whose location the caller
/// therefore has to record itself.
bool attach( Item& item, const Location& to )
{
  if ( to.holds<InWorld>() )
  {
    // An item on the ground has no slot: Location::slot() says so, and leaving the container's
    // index behind is the kind of residue that makes the two disagree.
    item.reset_slot();
    Core::add_item_to_world( &item );
    Core::register_with_supporting_multi( &item );
  }
  else if ( const auto* in_cont = to.get_if<InContainer>() )
  {
    item.slot_index( in_cont->slot );
    in_cont->cont->add( &item, in_cont->grid );
  }
  else if ( const auto* equipped = to.get_if<Equipped>() )
    equipped->chr->equip( &item );
  else if ( const auto* on_corpse = to.get_if<OnCorpse>() )
  {
    item.slot_index( on_corpse->slot );
    if ( on_corpse->layer != 0 )
      on_corpse->corpse->equip_and_add( &item, on_corpse->layer, on_corpse->grid );
    else
      on_corpse->corpse->add( &item, on_corpse->grid );
  }
  else if ( const auto* in_storage = to.get_if<InStorage>() )
    in_storage->area->insert_root_item( &item );
  else if ( to.holds<Destroyed>() )
    item.destroy();  // Destroyed is derived from the serial, so there is nothing to record
  else
    return false;  // Detached, Intrinsic, Absorbed

  return true;
}
}  // namespace

bool relocate( Item& item, Location to )
{
  const Location from = item.location();
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

  if ( to_cursor != nullptr )
    attach_to_cursor( item, *to_cursor->holder, ticket );
  else if ( !attach( item, to ) )
    item.set_location( to );

  return true;
}
}  // namespace Pol::Items
