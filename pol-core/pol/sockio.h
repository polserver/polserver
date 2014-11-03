/*
History
=======


Notes
=======

*/

#ifndef __SOCKIO_H
#define __SOCKIO_H

#include "sockets.h"

namespace Pol {
  namespace Network {
	extern char ipaddr_str[64];
	extern char lanaddr_str[64];

	int init_sockets_library( void );
	int deinit_sockets_library( void );

	SOCKET open_listen_socket( unsigned short port );
	void apply_socket_options( SOCKET client_socket );
	void disable_nagle (SOCKET client_socket );

	// These probably don't belong here, but better than POL.CPP.
	class Client;

	void transmit( Client *client, const void *data, int len );

	const char *AddressToString( struct sockaddr *addr );

  }
}
#endif
