/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"

#include <stdio.h>
#include <string.h>

#include "sockets.h"

#include "clib/endian.h"
#include "clib/clib.h"
#include "clib/strutil.h"

#include "sockio.h"

#ifdef __unix__
#include <sys/utsname.h>
struct utsname my_utsname;
#endif

char hostname[ 64 ];
char ipaddr_str[64];
char lanaddr_str[64];
void set_ip_address( const char* ip )
{
    strzcpy( ipaddr_str, ip, sizeof ipaddr_str );
    cout << "Internet IP address is " << ipaddr_str << endl;
}
void set_lan_address( const char* ip )
{
    strzcpy( lanaddr_str, ip, sizeof lanaddr_str );
    cout << "LAN IP address is " << lanaddr_str << endl;
}

void search_name( const char* hostname )
{
	struct		sockaddr_in server;
    cout << "hostname is " << hostname << endl;
    struct hostent* he = gethostbyname( hostname );
    for (int i = 0; (he != NULL) && (he->h_addr_list[i] != NULL); ++i )
    {
		memcpy( &server.sin_addr, he->h_addr_list[i], he->h_length);
    
		const char* adstr	= inet_ntoa( (struct in_addr)server.sin_addr );
        cout << "address: " << adstr << endl;
        
        if (strncmp( adstr, "192.168.", 8 ) == 0 ||
            strncmp( adstr, "10.", 3 ) == 0 ||
            strncmp( adstr, "172.16.", 7 ) == 0 ||
            strncmp( adstr, "172.32.", 7 ) == 0 )
        {
            if (!lanaddr_str[0])
                set_lan_address( adstr );
        }
        else if (strncmp( adstr, "127.", 4 ) == 0) 
        {
            ;
        }
        else
        {
            if (!ipaddr_str[0])
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

	res = WSAStartup( WSOCK_VERSION, &wsa_data);
	if (res < 0)
	{
		cout << "Error starting Winsock 1.1: " << res << endl;
		return -1;
	}
#endif

    if (gethostname( hostname, sizeof hostname ))
    {
        cout << "gethostname failed: " << socket_errno << endl;
    }
    search_name( hostname );

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
	if (res < 0)
	{
		cout << "Error stopping Winsock 1.1: " << res << endl;
		return -1;
	}
#endif
	return 0;
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
	if (res < 0)
	{
		throw runtime_error( "Unable to set socket to nonblocking mode, res=" + decint(res) );
	}
}
SOCKET open_listen_socket( unsigned short port )
{
	int res;
	SOCKET sck;

	sck = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if (sck < 0)
	{
		throw runtime_error( "Unable to create listening socket" );
		return -1;
	}

    apply_socket_options( sck );

#ifndef WIN32
	int reuse_opt = 1;
	res = setsockopt( sck, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse_opt, sizeof(reuse_opt) );
	if (res < 0)
	{
		throw runtime_error( "Unable to setsockopt (SO_REUSEADDR) on listening socket, res = " + decint(res) );
	}
#endif

#define DISABLE_NAGLE_ALGORITHM 0
#if DISABLE_NAGLE_ALGORITHM
    int tcp_nodelay = -1;
    res = setsockopt( sck, IPPROTO_TCP, TCP_NODELAY, (const char *) &tcp_nodelay, sizeof(tcp_nodelay) );
    if (res < 0)
    {
		cout << "Unable to setsockopt (TCP_NODELAY) on listening socket, res=" << res << endl;
		return -1;
    }
#endif

	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	connection.sin_addr.s_addr = INADDR_ANY;
	connection.sin_port = ctBEu16( port );


	res = bind( sck, (struct sockaddr *) &connection, sizeof connection );
	if ( res < 0 )
	{
		// Aug. 16, 2006. Austin
		//   Added the port number that failed.
		string tmp_error = "Unable to bind listening socket. Port("+decint(port)+") Res="+decint(res);
		throw runtime_error(tmp_error);
	}

	res = listen( sck, SOMAXCONN );
	if (res < 0)
	{
		throw runtime_error( "Listen failed, res=" + decint(res) );
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
	struct sockaddr_in *in_addr = (struct sockaddr_in *) addr;
	if (addr->sa_family == AF_INET)
		return inet_ntoa( in_addr->sin_addr );
	else
		return "(display error)";
#endif
}
