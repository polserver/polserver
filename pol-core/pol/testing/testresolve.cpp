/** @file
 *
 * @par History
 */

#include <algorithm>
#include <string>
#include <vector>

#include "clib/network/resolve.h"
#include "pol/testing/testenv.h"


namespace Pol::Testing
{
namespace
{
// dotted quad -> host-order address, so the cases below read like the CIDR blocks they test
constexpr Clib::ipv4_addr addr( unsigned a, unsigned b, unsigned c, unsigned d )
{
  return ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
}

bool resolves_to( const std::string& host, const std::string& expected )
{
  const auto addrs = Clib::resolve_ipv4( host );
  return std::any_of( addrs.begin(), addrs.end(),
                      [&]( Clib::ipv4_addr a ) { return Clib::ipv4_to_string( a ) == expected; } );
}
}  // namespace

void resolve_test()
{
#define T_PRIVATE( a, b, c, d, expected )                                           \
  UnitTest( []() { return Clib::is_private_ipv4( addr( a, b, c, d ) ); }, expected, \
            " is_private_ipv4( " #a "." #b "." #c "." #d " ) == " #expected )
#define T_LOOPBACK( a, b, c, d, expected )                                           \
  UnitTest( []() { return Clib::is_loopback_ipv4( addr( a, b, c, d ) ); }, expected, \
            " is_loopback_ipv4( " #a "." #b "." #c "." #d " ) == " #expected )

  // 10.0.0.0/8
  T_PRIVATE( 10, 0, 0, 0, true );
  T_PRIVATE( 10, 255, 255, 255, true );
  T_PRIVATE( 11, 0, 0, 1, false );
  T_PRIVATE( 9, 255, 255, 255, false );

  // 172.16.0.0/12 -- the edges are what the old byte-reversed bitmask made hard to check
  T_PRIVATE( 172, 15, 255, 255, false );
  T_PRIVATE( 172, 16, 0, 0, true );
  T_PRIVATE( 172, 31, 255, 255, true );
  T_PRIVATE( 172, 32, 0, 0, false );

  // 192.168.0.0/16
  T_PRIVATE( 192, 167, 255, 255, false );
  T_PRIVATE( 192, 168, 0, 0, true );
  T_PRIVATE( 192, 168, 255, 255, true );
  T_PRIVATE( 192, 169, 0, 0, false );

  // loopback is not private, and public addresses are neither
  T_PRIVATE( 127, 0, 0, 1, false );
  T_PRIVATE( 8, 8, 8, 8, false );
  T_PRIVATE( 203, 0, 113, 7, false );

  // the whole 127/8 block is loopback
  T_LOOPBACK( 127, 0, 0, 1, true );
  T_LOOPBACK( 127, 255, 255, 254, true );
  T_LOOPBACK( 126, 0, 0, 1, false );
  T_LOOPBACK( 128, 0, 0, 1, false );
  T_LOOPBACK( 10, 0, 0, 1, false );

#undef T_LOOPBACK
#undef T_PRIVATE

  UnitTest( []() { return Clib::ipv4_to_string( addr( 192, 168, 0, 1 ) ); },
            std::string( "192.168.0.1" ), " ipv4_to_string round-trips a dotted quad" );

  // a literal address must pass through the resolver unchanged
  UnitTest( []() { return resolves_to( "127.0.0.1", "127.0.0.1" ); }, true,
            " resolve_ipv4( \"127.0.0.1\" ) yields 127.0.0.1" );
  UnitTest( []() { return resolves_to( "localhost", "127.0.0.1" ); }, true,
            " resolve_ipv4( \"localhost\" ) yields 127.0.0.1" );

  // a name that cannot resolve reports failure rather than a bogus address. .invalid is
  // reserved by RFC 2606 precisely so it can never be registered.
  UnitTest( []() { return Clib::resolve_ipv4( "pol-nonexistent.invalid" ).empty(); }, true,
            " resolve_ipv4 of an unresolvable name is empty" );
}

}  // namespace Pol::Testing
