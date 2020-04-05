/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/09/15 MuadDib:   Multi registration/unregistration support added.
 */


#include "core.h"

#include <stddef.h>

#include "../clib/compilerspecifics.h"
#include "../clib/rawtypes.h"
#include "fnsearch.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "mobile/charactr.h"
#include "multi/house.h"
#include "multi/multi.h"
#include "network/cgdata.h"
#include "network/client.h"
#include "realms/realm.h"
#include "ufunc.h"
#include "uworld.h"

namespace Pol
{
namespace Core
{
void SetSysTrayPopupText( const char* text );
#ifdef _WIN32
static Priority tipPriority = ToolTipPriorityNone;
#endif
void CoreSetSysTrayToolTip( const std::string& text, Priority priority )
{
#ifdef _WIN32
  if ( priority >= tipPriority )
  {
    tipPriority = priority;
    SetSysTrayPopupText( text.c_str() );
  }
#else
  (void)text;
  (void)priority;
#endif
}

bool move_character_to( Mobile::Character* chr, Pos4d newpos, int flags )
{
  // FIXME consider consolidating with similar code in CHARACTER.CPP
  short newz;
  Multi::UMulti* supporting_multi = nullptr;
  Items::Item* walkon_item = nullptr;
  short new_boost = 0;
  Pos4d oldpos = chr->pos();

  if ( flags & MOVEITEM_FORCELOCATION )
  {
    newpos.realm()->walkheight( newpos.xy(), newpos.z(), &newz, &supporting_multi, &walkon_item,
                                true, chr->movemode, &new_boost );
  }
  else
  {
    if ( !newpos.realm()->walkheight( chr, newpos.xy(), newpos.z(), &newz, &supporting_multi,
                                      &walkon_item, &new_boost ) )
    {
      return false;
    }
    newpos.z( static_cast<s8>( newz ) );
  }
  chr->set_dirty();

  if ( ( oldpos.realm() != nullptr ) && ( oldpos.realm() != newpos.realm() ) )
  {
    // Notify NPCs in the old realm that the player left the realm.
    oldpos.realm()->notify_left( *chr );

    send_remove_character_to_nearby( chr );
    if ( chr->client != nullptr )
      remove_objects_inrange( chr->client );
    chr->setposition( newpos );
    chr->realm_changed();
    chr->lastxyz = Pos3d( 0, 0, 0 );
  }
  else
  {
    chr->setposition( newpos );
    chr->lastxyz = oldpos.xyz();
  }
  MoveCharacterWorldPosition( oldpos, chr );

  chr->gradual_boost = new_boost;
  chr->position_changed();
  // FIXME: Need to add Walkon checks for multi right here if type is house.
  if ( supporting_multi != nullptr )
  {
    supporting_multi->register_object( chr );
    Multi::UHouse* this_house = supporting_multi->as_house();
    if ( chr->registered_house == 0 )
    {
      chr->registered_house = supporting_multi->serial;

      if ( this_house != nullptr )
        this_house->walk_on( chr );
    }
  }
  else
  {
    if ( chr->registered_house > 0 )
    {
      Multi::UMulti* multi = system_find_multi( chr->registered_house );
      if ( multi != nullptr )
      {
        multi->unregister_object( chr );
      }
      chr->registered_house = 0;
    }
  }

  // teleport( chr );
  if ( chr->has_active_client() )
  {
    passert_assume( chr->client !=
                    nullptr );  // tells compiler to assume this is true during static code analysis

    if ( oldpos.realm() != chr->realm() )
    {
      send_new_subserver( chr->client );
      send_owncreate( chr->client, chr );
    }
    send_goxyz( chr->client, chr );

    // send_goxyz seems to stop the weather.  This will force a refresh, if the client cooperates.
    chr->client->gd->weather_region = nullptr;
  }
  if ( chr->isa( UOBJ_CLASS::CLASS_NPC ) ||
       chr->client )  // dave 3/26/3 dont' tell moves of offline PCs
  {
    chr->tellmove();
  }
  if ( chr->has_active_client() )
  {
    send_objects_newly_inrange( chr->client );
    chr->check_light_region_change();
  }
  if ( walkon_item != nullptr )
  {
    walkon_item->walk_on( chr );
  }

  chr->lastxyz = chr->pos().xyz();

  return true;
}

/* For us to care, the item must:
   1) be directly under the current position
   2) have a "walk on" script
   */

Items::Item* find_walkon_item( ItemsVector& ivec, short z )
{
  for ( ItemsVector::const_iterator itr = ivec.begin(), end = ivec.end(); itr != end; ++itr )
  {
    Items::Item* item = ( *itr );
    if ( z == item->z() || z == item->z() + 1 )
    {
      if ( !item->itemdesc().walk_on_script.empty() )
      {
        return item;
      }
    }
  }
  return nullptr;
}
}  // namespace Core
}  // namespace Pol
