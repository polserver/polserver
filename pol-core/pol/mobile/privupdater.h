#ifndef PRIVUPDATER_H
#define PRIVUPDATER_H

#ifndef __CHARACTR_H
#include "charactr.h"
#endif

#ifndef H_NPC_H
// At some point, npc.h should be removed.
// The only dependence is to send leftarea/enteredarea events.
#include "npc.h"
#endif

#include "../../clib/passert.h"
#include "../ufunc.h"
#include "../uworld.h"

namespace Pol
{
namespace Mobile
{
// This class deal with the side effects of activating/revoking privileges
// it can be used as a watcher by instanciating or directly via the statics
class PrivUpdater
{
public:
  PrivUpdater( Character* chr, PRIV_FLAGS flag );
  ~PrivUpdater();

private:
  Character* chr_;
  PRIV_FLAGS flag_;
  bool oldvalue_;

public:
  static void on_change_see_hidden( Character* chr, bool enable );
  static void on_change_see_ghosts( Character* chr, bool enable );
  static void on_change_see_invis_items( Character* chr, bool enable );
  static void on_change_invul( Character* chr, bool enable );

private:
  static bool is_active_or_npc( Character* chr );

  // Helper functions for the loops
  static void enable_see_hidden( Character* in_range_chr, Character* chr );
  static void disable_see_hidden( Character* in_range_chr, Character* chr );

  static void enable_see_ghosts( Character* in_range_chr, Character* chr );
  static void disable_see_ghosts( Character* in_range_chr, Character* chr );

  static void enable_see_invis_items( Items::Item* in_range_item, Character* chr );
  static void disable_see_invis_items( Items::Item* in_range_item, Character* chr );

  static void enable_invul( Character* in_range_chr, Character* chr );
  static void disable_invul( Character* in_range_chr, Character* chr,
                             Network::HealthBarStatusUpdate& msg );
};


PrivUpdater::PrivUpdater( Character* chr, PRIV_FLAGS flag )
    : chr_( chr ),
      flag_( flag ),
      oldvalue_( chr_ != nullptr ? chr_->cached_settings.get( flag_ ) : false )
{
}

PrivUpdater::~PrivUpdater()
{
  if ( chr_ == nullptr )
    return;
  bool newvalue = chr_->cached_settings.get( flag_ );
  if ( oldvalue_ == newvalue )
    return;
  switch ( flag_ )
  {
  case PRIV_FLAGS::INVUL:
    on_change_invul( chr_, newvalue );
    break;
  case PRIV_FLAGS::SEE_GHOSTS:
    on_change_see_ghosts( chr_, newvalue );
    break;
  case PRIV_FLAGS::SEE_HIDDEN:
    on_change_see_hidden( chr_, newvalue );
    break;
  case PRIV_FLAGS::SEE_INVIS_ITEMS:
    on_change_see_invis_items( chr_, newvalue );
    break;
  default:
    passert_always( 0 );
    break;
  }
}

void PrivUpdater::on_change_see_hidden( Character* chr, bool enable )
{
  if ( !is_active_or_npc( chr ) )
    return;  // don't do useless work

  if ( enable )
  {
    Core::WorldIterator<Core::MobileFilter>::InRange( chr->pos(), chr->update_range(),
                                                      [&]( Character* zonechr )
                                                      { enable_see_hidden( zonechr, chr ); } );
  }
  else
  {
    Core::WorldIterator<Core::MobileFilter>::InRange( chr->pos(), chr->update_range(),
                                                      [&]( Character* zonechr )
                                                      { disable_see_hidden( zonechr, chr ); } );
  }
}

void PrivUpdater::on_change_see_ghosts( Character* chr, bool enable )
{
  if ( !is_active_or_npc( chr ) )
    return;  // don't do useless work

  if ( enable )
  {
    Core::WorldIterator<Core::MobileFilter>::InRange( chr->pos(), chr->update_range(),
                                                      [&]( Character* zonechr )
                                                      { enable_see_ghosts( zonechr, chr ); } );
  }
  else
  {
    Core::WorldIterator<Core::MobileFilter>::InRange( chr->pos(), chr->update_range(),
                                                      [&]( Character* zonechr )
                                                      { disable_see_ghosts( zonechr, chr ); } );
  }
}

void PrivUpdater::on_change_see_invis_items( Character* chr, bool enable )
{
  // NPCs don't care about invisible items, so we only need to check for active clients.
  if ( chr == nullptr || !chr->has_active_client() )
    return;

  if ( enable )
  {
    Core::WorldIterator<Core::ItemFilter>::InMaxVisualRange(
        chr,
        [&]( Items::Item* zoneitem )
        {
          if ( chr->in_visual_range( zoneitem ) )
            enable_see_invis_items( zoneitem, chr );
        } );
  }
  else
  {
    Core::WorldIterator<Core::ItemFilter>::InMaxVisualRange(
        chr,
        [&]( Items::Item* zoneitem )
        {
          if ( chr->in_visual_range( zoneitem ) )
            disable_see_invis_items( zoneitem, chr );
        } );
  }
}

void PrivUpdater::on_change_invul( Character* chr, bool enable )
{
  if ( !is_active_or_npc( chr ) )
    return;  // don't do useless work

  if ( enable )
  {
    Core::WorldIterator<Core::OnlinePlayerFilter>::InMaxVisualRange(
        chr,
        [&]( Character* zonechr )
        {
          if ( zonechr->in_visual_range( chr ) )
            enable_invul( zonechr, chr );
        } );
  }
  else
  {
    Network::HealthBarStatusUpdate msg( chr->serial_ext,
                                        Network::HealthBarStatusUpdate::Color::YELLOW, false );
    Core::WorldIterator<Core::OnlinePlayerFilter>::InMaxVisualRange(
        chr,
        [&]( Character* zonechr )
        {
          if ( zonechr->in_visual_range( chr ) )
            disable_invul( zonechr, chr, msg );
        } );
  }
}

bool PrivUpdater::is_active_or_npc( Character* chr )
{
  return chr != nullptr && ( chr->has_active_client() || chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) );
}

void PrivUpdater::enable_see_hidden( Character* in_range_chr, Character* chr )
{
  if ( in_range_chr->hidden() && in_range_chr != chr )
  {
    if ( chr->client )
      send_owncreate( chr->client, in_range_chr );
    else if ( chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
    {
      NPC* npc = static_cast<NPC*>( chr );
      if ( npc->can_accept_event( Core::EVID_ENTEREDAREA ) &&
           npc->can_accept_area_event_by( in_range_chr ) )
        npc->send_event( new Module::SourcedEvent( Core::EVID_ENTEREDAREA, in_range_chr ) );
    }
  }
}

void PrivUpdater::disable_see_hidden( Character* in_range_chr, Character* chr )
{
  if ( in_range_chr->hidden() && in_range_chr != chr )
  {
    if ( chr->client )
      send_remove_character( chr->client, in_range_chr );
    else if ( chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
    {
      NPC* npc = static_cast<NPC*>( chr );
      if ( npc->can_accept_event( Core::EVID_LEFTAREA ) &&
           npc->can_accept_area_event_by( in_range_chr ) )
      {
        npc->send_event( new Module::SourcedEvent( Core::EVID_LEFTAREA, in_range_chr ) );
      }
    }
  }
}

void PrivUpdater::enable_see_ghosts( Character* in_range_chr, Character* chr )
{
  if ( in_range_chr->dead() && in_range_chr != chr )
  {
    if ( chr->client )
      send_owncreate( chr->client, in_range_chr );
    else if ( chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
    {
      NPC* npc = static_cast<NPC*>( chr );
      if ( npc->can_accept_event( Core::EVID_ENTEREDAREA ) &&
           npc->can_accept_area_event_by( in_range_chr ) )
        npc->send_event( new Module::SourcedEvent( Core::EVID_ENTEREDAREA, in_range_chr ) );
    }
  }
}

void PrivUpdater::disable_see_ghosts( Character* in_range_chr, Character* chr )
{
  if ( in_range_chr->dead() && in_range_chr != chr )
  {
    if ( chr->client )
      send_remove_character( chr->client, in_range_chr );
    else if ( chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
    {
      NPC* npc = static_cast<NPC*>( chr );
      if ( npc->can_accept_event( Core::EVID_LEFTAREA ) )
        npc->send_event( new Module::SourcedEvent( Core::EVID_LEFTAREA, in_range_chr ) );
    }
  }
}

void PrivUpdater::enable_see_invis_items( Items::Item* in_range_item, Character* chr )
{
  if ( in_range_item->invisible() )
  {
    send_item( chr->client, in_range_item );
  }
}

void PrivUpdater::disable_see_invis_items( Items::Item* in_range_item, Character* chr )
{
  if ( in_range_item->invisible() )
  {
    send_remove_object( chr->client, in_range_item );
  }
}

void PrivUpdater::enable_invul( Character* in_range_chr, Character* chr )
{
  if ( in_range_chr != chr )
  {
    if ( in_range_chr->is_visible_to_me( chr ) )
    {
      send_owncreate( in_range_chr->client, chr );
    }
  }
  else
  {
    // If it's the same, it can't be a NPC and must be active because of the OnlinePlayerFilter,
    // so it tells itself.
    send_move( chr->client, chr );
  }
}

void PrivUpdater::disable_invul( Character* in_range_chr, Character* chr,
                                 Network::HealthBarStatusUpdate& msg )
{
  if ( in_range_chr != chr )
  {
    if ( in_range_chr->is_visible_to_me( chr ) )
    {
      send_owncreate( in_range_chr->client, chr );

      // Needs to update the healthbar, because send_owncreate only sends if invul() == true.
      msg.Send( in_range_chr->client );
    }
  }
  else
  {
    send_move( chr->client,
               chr );  // tells itself if player (same justification as in enable_invul)

    // Needs to update the healthbar to the player as well
    msg.Send( chr->client );
  }
}
}  // namespace Mobile
}  // namespace Pol

#endif
