/** @file
 *
 * @par History
 * - 2005/12/05 MuadDib:   Added check for invul_tag from ssopt. If not 1, don't add tag.
 * - 2009/11/19 Turley:    ssopt.core_sends_season & .core_handled_tags - Tomi
 */

#include "sngclick.h"

#include <cstdio>
#include <string>

#include "../clib/clib_endian.h"
#include "../clib/passert.h"
#include "../plib/uconst.h"
#include "containr.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/pktin.h"
#include "ufunc.h"
#include "ufuncstd.h"
#include "uobject.h"
#include "uworld.h"


namespace Pol
{
namespace Core
{
Items::Item* find_legal_singleclick_item( Mobile::Character* chr, u32 serial )
{
  Items::Item* item = find_legal_item( chr, serial );
  if ( item )
    return item;

  // search equipment of nearby mobiles
  unsigned short wxL, wyL, wxH, wyH;
  zone_convert( chr->x - RANGE_VISUAL, chr->y - RANGE_VISUAL, chr->realm, &wxL, &wyL );
  zone_convert( chr->x + RANGE_VISUAL, chr->y + RANGE_VISUAL, chr->realm, &wxH, &wyH );
  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
  {
    for ( unsigned short wy = wyL; wy <= wyH; ++wy )
    {
      for ( const auto& ochr : chr->realm->zone[wx][wy].characters )
      {
        Items::Item* _item = ochr->find_wornitem( serial );
        if ( _item != nullptr )
          return _item;
      }
      for ( const auto& ochr : chr->realm->zone[wx][wy].npcs )
      {
        Items::Item* _item = ochr->find_wornitem( serial );
        if ( _item != nullptr )
          return _item;
      }
    }
  }
  if ( chr->trade_container() )
  {
    item = chr->trade_container()->find( serial );
    if ( item )
      return item;
  }
  if ( chr->trading_with.get() && chr->trading_with->trade_container() )
  {
    item = chr->trading_with->trade_container()->find( serial );
    if ( item )
      return item;
  }

  return nullptr;
}

std::string create_nametags( Mobile::Character* chr )
{
  std::string tags;

  if ( chr->frozen() && ( settingsManager.ssopt.core_handled_tags & 0x2 ) )
    tags = "[frozen] ";
  if ( chr->paralyzed() && ( settingsManager.ssopt.core_handled_tags & 0x4 ) )
    tags += "[paralyzed] ";
  if ( chr->squelched() && ( settingsManager.ssopt.core_handled_tags & 0x8 ) )
    tags += "[squelched] ";
  if ( chr->deafened() && ( settingsManager.ssopt.core_handled_tags & 0x10 ) )
    tags += "[deafened] ";
  if ( chr->invul() && settingsManager.ssopt.invul_tag == 1 )
    tags += "[invulnerable]";

  return tags;
}

void singleclick( Network::Client* client, u32 serial )
{
  passert_always( client != nullptr && client->chr != nullptr );

  if ( IsCharacter( serial ) )
  {
    Mobile::Character* chr = nullptr;
    if ( serial == client->chr->serial )
      chr = client->chr;
    else
      chr = find_character( serial );

    if ( chr != nullptr && inrange( client->chr, chr ) &&
         !client->chr->is_concealed_from_me( chr ) )
    {
      if ( chr->has_title_guild() && ( settingsManager.ssopt.core_handled_tags & 0x1 ) )
        send_nametext( client, chr, "[" + chr->title_guild() + "]" );
      send_nametext( client, chr, chr->name() );

      std::string tags = create_nametags( chr );
      if ( !tags.empty() )
        send_nametext( client, chr, tags );
    }
  }
  else  // single clicked on an item
  {
    Items::Item* item = find_legal_singleclick_item( client->chr, serial );
    if ( item )
    {
      send_objdesc( client, item );
    }
  }
}

void handle_singleclick( Network::Client* client, PKTIN_09* msg )
{
  u32 serial = cfBEu32( msg->serial );
  if ( client && client->chr )
    singleclick( client, serial );
  // TODO: report if someone tries to use singleclick without a connected char (should have been
  // blocked)
}
}  // namespace Core
}  // namespace Pol
