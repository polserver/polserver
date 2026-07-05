/** @file
 *
 * @par History
 */


#ifndef __SOCKIO_H
#define __SOCKIO_H

#include "../clib/network/sockets.h"
#include <string>


namespace Pol::Network
{
int init_sockets_library();
int deinit_sockets_library();

// loopback_only binds to 127.0.0.1 instead of all interfaces
SOCKET open_listen_socket( unsigned short port, bool loopback_only = false );
void apply_socket_options( SOCKET client_socket );
void disable_nagle( SOCKET client_socket );

std::string AddressToString( const sockaddr* addr );

}  // namespace Pol::Network

#endif
