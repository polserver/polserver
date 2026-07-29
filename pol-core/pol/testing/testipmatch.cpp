/** @file
 *
 * @par History
 */

#include <cstring>
#include <string>
#include <vector>

#include "clib/network/sockets.h"
#include "pol/network/ipmatch.h"
#include "pol/testing/testenv.h"


namespace Pol::Testing
{
namespace
{
sockaddr peer( const char* dotted )
{
  sockaddr_in in;
  memset( &in, 0, sizeof in );
  in.sin_family = AF_INET;
  in.sin_addr.s_addr = inet_addr( dotted );

  sockaddr out;
  memcpy( &out, &in, sizeof out );
  return out;
}

bool hits( const std::string& filter, const char* address )
{
  return Network::IpMatch::parse( filter ).matches( peer( address ) );
}
}  // namespace

void ipmatch_test()
{
#define T_MATCH( filter, address, expected )                            \
  UnitTest( []() { return hits( filter, address ); }, expected,         \
            " " filter " vs " address " == " #expected )

  // a bare address matches only itself
  T_MATCH( "192.168.0.5", "192.168.0.5", true );
  T_MATCH( "192.168.0.5", "192.168.0.6", false );

  // the second half is a dotted netmask, not a prefix length
  T_MATCH( "192.168.0.0/255.255.255.0", "192.168.0.1", true );
  T_MATCH( "192.168.0.0/255.255.255.0", "192.168.0.255", true );
  T_MATCH( "192.168.0.0/255.255.255.0", "192.168.1.1", false );
  T_MATCH( "192.168.0.0/255.255.0.0", "192.168.1.1", true );
  T_MATCH( "10.0.0.0/255.0.0.0", "10.1.2.3", true );
  T_MATCH( "10.0.0.0/255.0.0.0", "11.1.2.3", false );

  // a zero mask matches everything, which is what an operator writing /0.0.0.0 asks for
  T_MATCH( "1.2.3.4/0.0.0.0", "203.0.113.9", true );

  // bits of the filter address outside the mask are ignored
  T_MATCH( "192.168.0.99/255.255.255.0", "192.168.0.1", true );

#undef T_MATCH

  // a default-constructed filter matches nothing: mask 0 would match everything, so the
  // address must be 0 too and only 0.0.0.0 can hit it
  UnitTest( []() { return Network::IpMatch().matches( peer( "192.168.0.1" ) ); }, false,
            " a default-constructed IpMatch does not match a real address" );

  // an empty list matches nothing; callers turn that into their own default
  UnitTest( []() { return Network::matches_any( {}, peer( "192.168.0.1" ) ); }, false,
            " matches_any over an empty list is false" );

  UnitTest(
      []()
      {
        const std::vector<Network::IpMatch> filters{
            Network::IpMatch::parse( "10.0.0.0/255.0.0.0" ),
            Network::IpMatch::parse( "192.168.0.0/255.255.0.0" ) };
        return Network::matches_any( filters, peer( "192.168.7.7" ) ) &&
               Network::matches_any( filters, peer( "10.9.9.9" ) ) &&
               !Network::matches_any( filters, peer( "172.16.0.1" ) );
      },
      true, " matches_any tries every filter in the list" );

  // Only IPv4 peers can match. proxy_check relies on this for an unset proxy address,
  // where the old code read the address bytes regardless of family.
  UnitTest(
      []()
      {
        sockaddr unset;
        memset( &unset, 0, sizeof unset );
        unset.sa_family = AF_UNSPEC;
        return Network::IpMatch::parse( "0.0.0.0/0.0.0.0" ).matches( unset );
      },
      false, " a non-IPv4 peer matches nothing, even through a zero mask" );
}

}  // namespace Pol::Testing
