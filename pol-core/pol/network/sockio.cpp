/** @file
 *
 * @par History
 */


#include "sockio.h"

#ifdef __unix__
#include <sys/utsname.h>

struct utsname my_utsname;
#endif

#include <cstdio>
#include <cstring>
#include <string>

#include "../clib/clib.h"
#include "../clib/logfacility.h"
#include "../clib/network/sockets.h"
#include "../clib/strutil.h"
#include "globals/network.h"


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

void search_name( const char* hostname )
{
  struct sockaddr_in server;
  POLLOG_INFOLN( "hostname is {}", hostname );
  struct hostent* he = gethostbyname( hostname );
  for ( int i = 0; ( he != nullptr ) && ( he->h_addr_list[i] != nullptr ); ++i )
  {
    memcpy( &server.sin_addr, he->h_addr_list[i], he->h_length );

    const in_addr ad = server.sin_addr;
    const char* adstr = inet_ntoa( ad );
    POLLOG_INFOLN( "address: {}", adstr );

#ifdef _WIN32
    const unsigned long ip = ad.S_un.S_addr;
#else
    const unsigned long ip = ad.s_addr;
#endif
    // Careful: IPs are reversed (i.e. 1.0.168.192)
    if ( ( ip & 0x0000ffff ) == 0x0000a8c0 ||  // 192.168.0.0/16
         ( ip & 0x0000f0ff ) == 0x000010ac ||  // 172.16.0.0/12
         ( ip & 0x000000ff ) == 0x0000000a )   // 10.0.0.0/8
    {
      if ( !Core::networkManager.lanaddr_str[0] )
        set_lan_address( adstr );
    }
    else if ( ( ip & 0x000000ff ) == 0x0000007f )  // 127.0.0.0/8
    {
      ;
    }
    else
    {
      if ( !Core::networkManager.ipaddr_str[0] )
        set_ip_address( adstr );
    }
  }
}


#ifdef _WIN32
#define WSOCK_VERSION 0x0101
WSADATA wsa_data;
#endif

int init_sockets_library()
{
#ifdef _WIN32
  int res;

  res = WSAStartup( WSOCK_VERSION, &wsa_data );
  if ( res < 0 )
  {
    POLLOG_ERRORLN( "Error starting Winsock 1.1: {}", res );
    return -1;
  }
#endif

  if ( gethostname( Core::networkManager.hostname, sizeof Core::networkManager.hostname ) )
  {
    POLLOG_ERRORLN( "gethostname failed: {}", socket_errno );
  }
  search_name( Core::networkManager.hostname );

#ifdef __unix__
  uname( &my_utsname );
  search_name( my_utsname.nodename );
#endif

  return 0;
}


int deinit_sockets_library()
{
#ifdef _WIN32
  int res;

  res = WSACleanup();
  if ( res < 0 )
  {
    POLLOG_ERRORLN( "Error stopping Winsock 1.1: {}", res );
    return -1;
  }
#endif
  return 0;
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
    return std::string( "(error - " + std::to_string( socket_errno ) + ")" );

  return address;
}

}  // namespace Pol::Network
