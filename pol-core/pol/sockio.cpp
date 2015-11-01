/*
History
=======


Notes
=======

*/


#include "sockio.h"

#include "sockets.h"

#include "../clib/clib_endian.h"
#include "../clib/clib.h"
#include "../clib/strutil.h"
#include "../clib/logfacility.h"

#include "globals/network.h"

#ifdef __unix__
#include <sys/utsname.h>
struct utsname my_utsname;
#endif

#include <cstdio>
#include <cstring>

namespace Pol {
  namespace Network {
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
	  struct		sockaddr_in server;
      POLLOG_INFO << "hostname is " << hostname << "\n";
	  struct hostent* he = gethostbyname( hostname );
	  for ( int i = 0; ( he != NULL ) && ( he->h_addr_list[i] != NULL ); ++i )
	  {
		memcpy( &server.sin_addr, he->h_addr_list[i], he->h_length );

		const char* adstr = inet_ntoa( ( struct in_addr )server.sin_addr );
        POLLOG_INFO << "address: " << adstr << "\n";

		if ( strncmp( adstr, "192.168.", 8 ) == 0 ||
			 strncmp( adstr, "10.", 3 ) == 0 ||
			 strncmp( adstr, "172.16.", 7 ) == 0 ||
			 strncmp( adstr, "172.32.", 7 ) == 0 )
		{
		  if ( !Core::networkManager.lanaddr_str[0] )
			set_lan_address( adstr );
		}
		else if ( strncmp( adstr, "127.", 4 ) == 0 )
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
		int res = setsockopt( sck, IPPROTO_TCP, TCP_NODELAY, (const char *) &tcp_nodelay, sizeof(tcp_nodelay) );
		if (res < 0)
		{
            throw std::runtime_error("Unable to setsockopt (TCP_NODELAY) on listening socket, res=" + Clib::decint(res));
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
          throw std::runtime_error("Unable to set socket to nonblocking mode, res=" + Clib::decint(res));
	  }
	}
	SOCKET open_listen_socket( unsigned short port )
	{
	  int res;
	  SOCKET sck;

	  sck = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	  if ( sck == INVALID_SOCKET )
	  {
          throw std::runtime_error("Unable to create listening socket");
		return INVALID_SOCKET;
	  }

	  apply_socket_options( sck );

#ifndef WIN32
	  int reuse_opt = 1;
	  res = setsockopt( sck, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse_opt, sizeof(reuse_opt) );
	  if (res < 0)
	  {
          throw std::runtime_error( "Unable to setsockopt (SO_REUSEADDR) on listening socket, res = " + Clib::decint(res) );
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
      memset(connection.sin_zero, 0, sizeof(connection.sin_zero)); // for completeness

	  res = bind( sck, ( struct sockaddr * ) &connection, sizeof connection );
	  if ( res < 0 )
	  {
		// Aug. 16, 2006. Austin
		//   Added the port number that failed.
		std::string tmp_error = "Unable to bind listening socket. Port(" + Clib::decint( port ) + ") Res=" + Clib::decint( res );
		throw std::runtime_error( tmp_error );
	  }

	  res = listen( sck, SOMAXCONN );
	  if ( res < 0 )
	  {
          throw std::runtime_error("Listen failed, res=" + Clib::decint(res));
	  }

	  return sck;
	}

	const char *AddressToString( struct sockaddr *addr )
	{
#if 0 && defined(_WIN32)
	  // this requires Winsock 2! Ouch.
	  static char buf[ 80 ];
	  DWORD len = sizeof buf;
	  if (WSAAddressToString( addr, sizeof *addr, 
		NULL, // protocol
		buf, &len ) != SOCKET_ERROR)
	  {
		return buf;
	  }
	  else
	  {
		return "(display error)";
	  }
#else
	  struct sockaddr_in *in_addr = ( struct sockaddr_in * ) addr;
	  if ( addr->sa_family == AF_INET )
		return inet_ntoa( in_addr->sin_addr );
	  else
		return "(display error)";
#endif
	}

	PolSocket::PolSocket() :
	  listen_timeout( { 0, 0 } ),
	  select_timeout ({ 0, 0 })
	{}
  }
}