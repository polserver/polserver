/** @file
 *
 * @par History
 */


#include "pol/network/sockio.h"

#ifdef __unix__
#include <sys/utsname.h>

struct utsname my_utsname;
#endif

#include <cstdio>
#include <cstring>
#include <string>

#include "clib/clib.h"
#include "clib/logfacility.h"
#include "clib/network/resolve.h"
#include "clib/network/sockets.h"
#include "clib/network/wnsckt.h"
#include "clib/strutil.h"
#include "pol/globals/network.h"


namespace Pol::Network
{
void set_ip_address( const char* ip )
{
  Clib::stracpy( Core::networkManager.ipaddr_str, ip, sizeof Core::networkManager.ipaddr_str );
  POLLOG_INFOLN( "Internet IP address is {}", Core::networkManager.ipaddr_str );
}
void set_lan_address( const char* ip )
{
  Clib::stracpy( Core::networkManager.lanaddr_str, ip, sizeof Core::networkManager.lanaddr_str );
  POLLOG_INFOLN( "LAN IP address is {}", Core::networkManager.lanaddr_str );
}

// Records this host's own addresses for SERVERS.CFG's --lan-- and --ip-- placeholders:
// the first private address found becomes the LAN address, the first public one the
// internet address. Loopback counts as neither. What callers depend on is the
// classification, not which address of a class gets picked -- a multi-homed host may
// well be offered its addresses in a different order than it was before.
void search_name( const char* hostname )
{
  POLLOG_INFOLN( "hostname is {}", hostname );

  for ( const Clib::ipv4_addr addr : Clib::resolve_ipv4( hostname ) )
  {
    const std::string adstr = Clib::ipv4_to_string( addr );
    POLLOG_INFOLN( "address: {}", adstr );

    if ( Clib::is_loopback_ipv4( addr ) )
      continue;

    if ( Clib::is_private_ipv4( addr ) )
    {
      if ( !Core::networkManager.lanaddr_str[0] )
        set_lan_address( adstr.c_str() );
    }
    else if ( !Core::networkManager.ipaddr_str[0] )
    {
      set_ip_address( adstr.c_str() );
    }
  }
}


void init_sockets_library()
{
  // Must come first: the lookups below are socket-library calls, which fail on Windows
  // until it is up. Nothing has constructed a Clib::Socket this early in startup, so this
  // is the one place that has to ask for it directly. Getting the order wrong is quiet
  // rather than loud -- the lookup just fails, ipaddr_str stays empty, and every
  // `IP --ip--` game server disappears from the UO client's server list (uimport.cpp).
  Clib::winsock_initialize();

  if ( gethostname( Core::networkManager.hostname, sizeof Core::networkManager.hostname ) )
  {
    POLLOG_ERRORLN( "gethostname failed: {}", Clib::socket_errno() );
  }
  search_name( Core::networkManager.hostname );

#ifdef __unix__
  uname( &my_utsname );
  search_name( my_utsname.nodename );
#endif
}

std::string AddressToString( const sockaddr* addr )
{
  if ( addr == nullptr )
    return "(no address)";

  if ( addr->sa_family != AF_INET )
    return std::string( "(unknown address family " ) + std::to_string( addr->sa_family ) + ")";

  const sockaddr_in* in_addr = reinterpret_cast<const sockaddr_in*>( addr );

  char address[INET_ADDRSTRLEN] = {};
  if ( inet_ntop( AF_INET, reinterpret_cast<const void*>( &in_addr->sin_addr ), address,
                  sizeof( address ) ) == nullptr )
    return std::string( "(error - " + std::to_string( Clib::socket_errno() ) + ")" );

  return address;
}

}  // namespace Pol::Network
