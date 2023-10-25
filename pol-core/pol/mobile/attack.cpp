/** @file
 *
 * @par History
 * - 2005/11/26 MuadDib:   Added update code for updating highlight packet to client.
 * - 2005/11/26 Shinigami: added Character check
 * - 2005/11/25 MuadDib:   Added distance check in handle_attack.
 * - 2006/03/07 MuadDib:   Added Justice Region NoCombat check to attack request.
 * - 2009/01/03 MuadDib:   Removed logging of null defenders. Who cares if it's empty :o
 */


#include <cstdio>

#include "../../clib/clib_endian.h"
#include "../../clib/rawtypes.h"
#include "../cmbtcfg.h"
#include "../globals/settings.h"
#include "../network/client.h"
#include "../network/pktin.h"
#include "../ufunc.h"
#include "charactr.h"
#include "regions/guardrgn.h"

namespace Pol
{
namespace Mobile
{
void handle_attack( Network::Client* client, Core::PKTIN_05* msg )
{
  if ( client->chr->dead() )
  {
    private_say_above( client->chr, client->chr, "I am dead and cannot do that." );
    return;
  }

  u32 serial = cfBEu32( msg->serial );
  Character* defender = Core::find_character( serial );
  if ( defender == nullptr )
    return;
  if ( !( Core::settingsManager.combat_config.attack_self ) )
  {
    if ( defender->serial == client->chr->serial )
    {
      client->chr->send_highlight();
      return;
    }
  }

  if ( !client->chr->is_visible_to_me( defender ) )
  {
    client->chr->send_highlight();
    return;
  }
  if ( !client->chr->in_visual_range( defender ) )
  {
    client->chr->send_highlight();
    return;
  }

  if ( defender->acct != nullptr )
  {
    if ( Core::JusticeRegion::RunNoCombatCheck( defender->client ) == true )
    {
      client->chr->send_highlight();
      Core::send_sysmessage( client, "Combat is not allowed in this area." );
      return;
    }
  }
  client->chr->select_opponent( serial );
}
}  // namespace Mobile
}  // namespace Pol
