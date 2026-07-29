#ifndef CLIB_RESOLVE_H
#define CLIB_RESOLVE_H

#include <cstdint>
#include <string>
#include <vector>

namespace Pol::Clib
{
// An IPv4 address in host byte order, so the classifiers below can be written the way the
// CIDR blocks are usually spelled instead of as byte-reversed bitmasks.
using ipv4_addr = std::uint32_t;

// Every IPv4 address `host` resolves to, in the order the resolver returned them; empty
// when the name does not resolve. `host` may be a name or a dotted quad.
//
// Blocks for as long as the resolver takes. Callers on latency-sensitive paths should
// care about that; none of them currently resolve asynchronously.
std::vector<ipv4_addr> resolve_ipv4( const std::string& host );

// Dotted quad, e.g. "192.168.0.1". Empty if the address cannot be formatted.
std::string ipv4_to_string( ipv4_addr addr );

// The whole 127.0.0.0/8 block is loopback, not just 127.0.0.1.
constexpr bool is_loopback_ipv4( ipv4_addr addr )
{
  return ( addr >> 24 ) == 127;
}

// The RFC 1918 private ranges: 10/8, 172.16/12, 192.168/16. Loopback is not one of them --
// callers that want "not routable from the internet" must test both.
constexpr bool is_private_ipv4( ipv4_addr addr )
{
  return ( addr >> 24 ) == 10 || ( addr >> 20 ) == 0xAC1 || ( addr >> 16 ) == 0xC0A8;
}
}  // namespace Pol::Clib

#endif  // CLIB_RESOLVE_H
