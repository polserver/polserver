/** @file
 *
 * @par History
 */


#ifndef __SOCKIO_H
#define __SOCKIO_H

#include "clib/network/sockets.h"
#include <string>


namespace Pol::Network
{
// Brings up the socket library and discovers this host's own internet/LAN addresses,
// which the SERVERS.CFG loader hands to UO clients. There is no matching deinit: the
// socket library tears itself down at exit (Clib::winsock_initialize).
void init_sockets_library();

std::string AddressToString( const sockaddr* addr );

}  // namespace Pol::Network

#endif
