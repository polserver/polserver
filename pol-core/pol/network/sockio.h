/** @file
 *
 * @par History
 */


#ifndef __SOCKIO_H
#define __SOCKIO_H

#include "../clib/network/sockets.h"
#include <string>

namespace Pol
{
namespace Network
{
int init_sockets_library( void );
int deinit_sockets_library( void );

SOCKET open_listen_socket( unsigned short port );
void apply_socket_options( SOCKET client_socket );
void disable_nagle( SOCKET client_socket );

std::string AddressToString( const sockaddr* addr );

}  // namespace Network
}  // namespace Pol
#endif
