/** @file
 *
 * @par History
 */


#ifndef SERVDESC_H
#define SERVDESC_H

#include <string>
#include <vector>

#include "clib/network/resolve.h"

namespace Pol::Core
{
class ServerDescription
{
public:
  ServerDescription();
  size_t estimateSize() const;

  std::string name;
  unsigned char ip[4];
  unsigned short port;
  std::vector<unsigned int> ip_match;
  std::vector<unsigned int> ip_match_mask;
  std::vector<unsigned int> proxy_match;
  std::vector<unsigned int> proxy_match_mask;
  std::vector<std::string> acct_match;
  std::string hostname;
};

// Both the server list (0xA8) and the relay (0x8C) carry the address least significant
// byte first, so 1.2.3.4 goes on the wire as { 4, 3, 2, 1 }.
inline void pack_server_ip( unsigned char ( &ip )[4], Clib::ipv4_addr addr )
{
  for ( int i = 0; i < 4; ++i )
    ip[i] = static_cast<unsigned char>( ( addr >> ( 8 * i ) ) & 0xFF );
}
}  // namespace Pol::Core

#endif
