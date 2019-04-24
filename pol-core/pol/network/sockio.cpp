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
#include <fcntl.h>
#include <stdexcept>
#include <string>

#include "../clib/clib.h"
#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/strutil.h"
#include "globals/network.h"
#include "sockets.h"


namespace Pol
{
namespace Network
{
void set_ip_address( const char* ip )
{
  strzcpy( Core::networkManager.ipaddr_str, ip, sizeof Core::networkManager.ipaddr_str );
  POLLOG_INFO << "Internet IP address is " << Core::networkManager.ipaddr_str << "\n";
}
void set_lan_address( const char* ip )
{
  strzcpy( Core::networkManager.lanaddr_str, ip, sizeof Core::networkManager.lanaddr_str );
  POLLOG_INFO << "LAN IP address is " << Core::networkManager.lanaddr_str << "\n";
}

void search_name( const char* hostname )
{
  struct sockaddr_in server;
  POLLOG_INFO << "hostname is " << hostname << "\n";
  struct hostent* he = gethostbyname( hostname );
  for ( int i = 0; ( he != nullptr ) && ( he->h_addr_list[i] != nullptr ); ++i )
  {
    memcpy( &server.sin_addr, he->h_addr_list[i], he->h_length );

    const in_addr ad = server.sin_addr;
    const char* adstr = inet_ntoa( ad );
    POLLOG_INFO << "address: " << adstr << "\n";

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

int init_sockets_library( void )
{
#ifdef _WIN32
  int res;

  res = WSAStartup( WSOCK_VERSION, &wsa_data );
  if ( res < 0 )
  {
    POLLOG_ERROR << "Error starting Winsock 1.1: " << res << "\n";
    return -1;
  }
#endif

  if ( gethostname( Core::networkManager.hostname, sizeof Core::networkManager.hostname ) )
  {
    POLLOG_ERROR << "gethostname failed: " << socket_errno << "\n";
  }
  search_name( Core::networkManager.hostname );

#ifdef __unix__
  uname( &my_utsname );
  search_name( my_utsname.nodename );
#endif

  return 0;
}


int deinit_sockets_library( void )
{
#ifdef _WIN32
  int res;

  res = WSACleanup();
  if ( res < 0 )
  {
    POLLOG_ERROR << "Error stopping Winsock 1.1: " << res << "\n";
    return -1;
  }
#endif
  return 0;
}

void disable_nagle( SOCKET sck )
{
  int tcp_nodelay = 1;
  int res =
      setsockopt( sck, IPPROTO_TCP, TCP_NODELAY, (const char*)&tcp_nodelay, sizeof( tcp_nodelay ) );
  if ( res < 0 )
  {
    throw std::runtime_error( "Unable to setsockopt (TCP_NODELAY) on listening socket, res=" +
                              Clib::tostring( res ) );
  }
}

void apply_socket_options( SOCKET sck )
{
#ifdef _WIN32
  u_long nonblocking = 1;
  int res = ioctlsocket( sck, FIONBIO, &nonblocking );
#else
  int flags = fcntl( sck, F_GETFL );
  flags |= O_NONBLOCK;
  int res = fcntl( sck, F_SETFL, flags );
#endif
  if ( res < 0 )
  {
    throw std::runtime_error( "Unable to set socket to nonblocking mode, res=" +
                              Clib::tostring( res ) );
  }
}
SOCKET open_listen_socket( unsigned short port )
{
  int res;
  SOCKET sck;

  sck = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( sck == INVALID_SOCKET )
  {
    throw std::runtime_error( "Unable to create listening socket" );
  }

  apply_socket_options( sck );

#ifndef WIN32
  int reuse_opt = 1;
  res = setsockopt( sck, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_opt, sizeof( reuse_opt ) );
  if ( res < 0 )
  {
    throw std::runtime_error( "Unable to setsockopt (SO_REUSEADDR) on listening socket, res = " +
                              Clib::tostring( res ) );
  }
#endif

#define DISABLE_NAGLE_ALGORITHM 0
#if DISABLE_NAGLE_ALGORITHM
  disable_nagle( sck );
#endif

  struct sockaddr_in connection;
  connection.sin_family = AF_INET;
  connection.sin_addr.s_addr = INADDR_ANY;
  connection.sin_port = ctBEu16( port );
  memset( connection.sin_zero, 0, sizeof( connection.sin_zero ) );  // for completeness

  res = bind( sck, (struct sockaddr*)&connection, sizeof connection );
  if ( res < 0 )
  {
    // Aug. 16, 2006. Austin
    //   Added the port number that failed.
    std::string tmp_error = "Unable to bind listening socket. Port(" + Clib::tostring( port ) +
                            ") Res=" + Clib::tostring( res );
    throw std::runtime_error( tmp_error );
  }

  res = listen( sck, SOMAXCONN );
  if ( res < 0 )
  {
    throw std::runtime_error( "Listen failed, res=" + Clib::tostring( res ) );
  }

  return sck;
}

std::string AddressToString( struct sockaddr* addr )
{
  struct sockaddr_in* in_addr = (struct sockaddr_in*)addr;
  if ( addr->sa_family == AF_INET )
    return inet_ntoa( in_addr->sin_addr );
  else
    return std::string( "(unknown address family " ) + std::to_string( addr->sa_family ) + ")";
}

PolSocket::PolSocket()
    : listen_socket( INVALID_SOCKET ),
      listen_fd(),
      listen_timeout( {0, 0} ),
      recv_fd(),
      err_fd(),
      send_fd(),
      select_timeout( {0, 0} )
{
  FD_ZERO( &listen_fd );
  FD_ZERO( &recv_fd );
  FD_ZERO( &err_fd );
  FD_ZERO( &send_fd );
}
}  // namespace Network
}  // namespace Pol
