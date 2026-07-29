/** @file
 *
 * @par History
 */


#include "pol/network/bannedips.h"

#include <string>

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "pol/globals/network.h"
#include "pol/network/client.h"
#include "pol/network/ipmatch.h"


namespace Pol::Network
{
bool is_banned_ip( Client* client )
{
  return matches_any( Core::networkManager.banned_ips, client->ipaddr );
}

void read_bannedips_config( bool initial_load )
{
  if ( !initial_load )
  {
    Core::networkManager.banned_ips.clear();
  }
  if ( !Clib::FileExists( "config/bannedips.cfg" ) )
    return;

  Clib::ConfigFile cf( "config/bannedips.cfg" );
  Clib::ConfigElem elem;

  while ( cf.read( elem ) )
  {
    Core::networkManager.banned_ips.push_back(
        IpMatch::parse( elem.remove_string( "IPMatch" ) ) );
  }
}
}  // namespace Pol::Network
