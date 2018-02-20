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

bool move_character_to( Mobile::Character* chr, unsigned short x, unsigned short y, short z,
                        int flags, Realms::Realm* oldrealm )
{
  // FIXME consider consolidating with similar code in CHARACTER.CPP
  short newz;
  Multi::UMulti* supporting_multi = NULL;
  Items::Item* walkon_item = NULL;
  short new_boost = 0;

  if ( flags & MOVEITEM_FORCELOCATION )
  {
    if ( x >= chr->realm->width() || y >= chr->realm->height() )
    {
      return false;
    }

    chr->realm->walkheight( x, y, z, &newz, &supporting_multi, &walkon_item, true, chr->movemode,
                            &new_boost );
    newz = z;
  }
  else
  {
    if ( !chr->realm->walkheight( chr, x, y, z, &newz, &supporting_multi, &walkon_item,
                                  &new_boost ) )
    {
      return false;
    }
  }
  chr->set_dirty();

  if ( ( oldrealm != NULL ) && ( oldrealm != chr->realm ) )
  {
    chr->lastx = 0;
    chr->lasty = 0;
    chr->lastz = 0;
  }
  else
  {
    chr->lastx = chr->x;
    chr->lasty = chr->y;
    chr->lastz = chr->z;
  }

  MoveCharacterWorldPosition( chr->x, chr->y, x, y, chr, oldrealm );
  chr->x = x;
  chr->y = y;
  chr->z = static_cast<s8>( newz );

  chr->gradual_boost = new_boost;
  chr->position_changed();
  // FIXME: Need to add Walkon checks for multi right here if type is house.
  if ( supporting_multi != NULL )
  {
    supporting_multi->register_object( chr );
    Multi::UHouse* this_house = supporting_multi->as_house();
    if ( chr->registered_house == 0 )
    {
      chr->registered_house = supporting_multi->serial;

      if ( this_house != NULL )
        this_house->walk_on( chr );
    }
  }
  else
  {
    if ( chr->registered_house > 0 )
    {
      Multi::UMulti* multi = system_find_multi( chr->registered_house );
      if ( multi != NULL )
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
                    NULL );  // tells compiler to assume this is true during static code analysis

    if ( oldrealm != chr->realm )
    {
      send_new_subserver( chr->client );
      send_owncreate( chr->client, chr );
    }
    send_goxyz( chr->client, chr );

    // send_goxyz seems to stop the weather.  This will force a refresh, if the client cooperates.
    chr->client->gd->weather_region = NULL;
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
  if ( walkon_item != NULL )
  {
    walkon_item->walk_on( chr );
  }

  chr->lastx = chr->x;
  chr->lasty = chr->y;
  chr->lastz = chr->z;

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
    if ( z == item->z || z == item->z + 1 )
    {
      if ( !item->itemdesc().walk_on_script.empty() )
      {
        return item;
      }
    }
  }
  return NULL;
}
}
}
