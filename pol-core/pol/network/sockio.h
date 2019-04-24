/** @file
 *
 * @par History
 */


#ifndef __SOCKIO_H
#define __SOCKIO_H

#include "sockets.h"
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

// These probably don't belong here, but better than POL.CPP.
class Client;

void transmit( Client* client, const void* data, int len );

std::string AddressToString( struct sockaddr* addr );

struct PolSocket
{
  PolSocket();
  SOCKET listen_socket;
  fd_set listen_fd;
  struct timeval listen_timeout;

  fd_set recv_fd;
  fd_set err_fd;
  fd_set send_fd;
  struct timeval select_timeout;
};
}  // namespace Network
}  // namespace Pol
#endif
