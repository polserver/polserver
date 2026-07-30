#include "clib/network/resolve.h"

#include <cstring>

#include "clib/network/sockets.h"
#include "clib/network/wnsckt.h"

namespace Pol::Clib
{
std::vector<ipv4_addr> resolve_ipv4( const std::string& host )
{
  // Callers resolve before they own a Socket, so nothing else has brought winsock up.
  winsock_initialize();

  struct addrinfo hints;
  memset( &hints, 0, sizeof hints );
  hints.ai_family = AF_INET;
  // Without a socktype the resolver returns the same address once per protocol; asking for
  // one gives a list of distinct addresses, which is what gethostbyname used to hand back.
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo* addrs = nullptr;
  if ( getaddrinfo( host.c_str(), nullptr, &hints, &addrs ) != 0 )
    return {};

  std::vector<ipv4_addr> result;
  for ( const struct addrinfo* addr = addrs; addr != nullptr; addr = addr->ai_next )
  {
    if ( addr->ai_family != AF_INET || addr->ai_addr == nullptr )
      continue;
    const auto* in = reinterpret_cast<const struct sockaddr_in*>( addr->ai_addr );
    result.push_back( ntohl( in->sin_addr.s_addr ) );
  }

  freeaddrinfo( addrs );
  return result;
}

std::string ipv4_to_string( ipv4_addr addr )
{
  struct in_addr in;
  memset( &in, 0, sizeof in );
  in.s_addr = htonl( addr );

  char buffer[INET_ADDRSTRLEN] = {};
  if ( inet_ntop( AF_INET, &in, buffer, sizeof buffer ) == nullptr )
    return {};
  return buffer;
}
}  // namespace Pol::Clib
