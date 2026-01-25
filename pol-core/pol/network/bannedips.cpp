/** @file
 *
 * @par History
 */


#include "bannedips.h"

#include <string>

#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/fileutil.h"
#include "../globals/network.h"
#include "client.h"


namespace Pol::Network
{
bool is_banned_ip( Client* client )
{
  if ( Core::networkManager.banned_ips.empty() )
    return false;

  for ( auto& banned_ip : Core::networkManager.banned_ips )
  {
    unsigned int addr1part, addr2part;
    struct sockaddr_in* sockin = reinterpret_cast<struct sockaddr_in*>( &client->ipaddr );
    addr1part = banned_ip.ipMatch & banned_ip.ipMask;

#ifdef _WIN32
    addr2part = sockin->sin_addr.S_un.S_addr & itr->ipMask;
#else
    addr2part = sockin->sin_addr.s_addr & banned_ip.ipMask;
#endif

    if ( addr1part == addr2part )
      return true;
  }
  return false;
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
    IPRule CurrentEntry;
    std::string iptext = elem.remove_string( "IPMatch" );
    std::string::size_type delim = iptext.find_first_of( '/' );
    if ( delim != std::string::npos )
    {
      std::string ipaddr_str = iptext.substr( 0, delim );
      std::string ipmask_str = iptext.substr( delim + 1 );
      CurrentEntry.ipMatch = inet_addr( ipaddr_str.c_str() );
      CurrentEntry.ipMask = inet_addr( ipmask_str.c_str() );
      Core::networkManager.banned_ips.push_back( CurrentEntry );
    }
    else
    {
      std::string ipmask_str = "255.255.255.255";
      CurrentEntry.ipMatch = inet_addr( iptext.c_str() );
      CurrentEntry.ipMask = inet_addr( ipmask_str.c_str() );
      Core::networkManager.banned_ips.push_back( CurrentEntry );
    }
  }
}
}  // namespace Pol::Network
