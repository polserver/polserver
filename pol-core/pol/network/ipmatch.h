#ifndef POL_IPMATCH_H
#define POL_IPMATCH_H

#include <string>
#include <vector>

#include "clib/network/sockets.h"

namespace Pol
{
namespace Clib
{
class ConfigElem;
}
namespace Network
{
// One entry of an address filter: an address plus the mask of the bits that have to agree.
// Four config settings are lists of these -- SERVERS.CFG IPMatch and ProxyMatch, an aux
// service's IPMatch, and bannedips.cfg.
//
// Addresses are held in network byte order, which is what inet_addr returns and what
// sockaddr_in::sin_addr stores, so matching is a masked compare with no conversion. The
// order does not matter to the comparison as long as both sides share it.
// There is deliberately no default constructor: a zero mask compares no bits at all, so a
// value-initialized filter would match every address. parse() is the only way to make one.
class IpMatch
{
public:
  // Config syntax: "a.b.c.d" for a single address, or "a.b.c.d/m.m.m.m" for a range. The
  // second half is a dotted netmask, not a prefix length -- 192.168.0.0/255.255.255.0,
  // never 192.168.0.0/24. Parsing is inet_addr's, which accepts short forms like "10.1"
  // and turns anything it cannot parse at all into 255.255.255.255.
  static IpMatch parse( const std::string& text );

  bool matches( const sockaddr& peer ) const;

private:
  IpMatch( unsigned int addr, unsigned int mask ) : _addr( addr ), _mask( mask ) {}

  unsigned int _addr;
  unsigned int _mask;
};

// An empty list matches nothing. What that means is the caller's policy, and it differs:
// a game server with no IPMatch is offered to every client, an empty bannedips.cfg bans
// nobody, an aux service with no IPMatch accepts anyone.
bool matches_any( const std::vector<IpMatch>& filters, const sockaddr& peer );

// Consumes every occurrence of `tag` in `elem`, appending one filter each.
void read_ip_match_list( Clib::ConfigElem& elem, const char* tag, std::vector<IpMatch>* filters );
}  // namespace Network
}  // namespace Pol

#endif  // POL_IPMATCH_H
