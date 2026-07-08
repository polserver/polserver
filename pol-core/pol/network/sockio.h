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

std::string AddressToString( const sockaddr* addr );

}  // namespace Pol::Network

#endif
