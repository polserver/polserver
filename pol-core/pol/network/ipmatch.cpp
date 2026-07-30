/** @file
 *
 * @par History
 */

#include "pol/network/ipmatch.h"

#include <algorithm>

#include "clib/cfgelem.h"


namespace Pol::Network
{
namespace
{
// no '/' in the config value means the address must match exactly
constexpr unsigned int exact_mask = 0xFFffFFffu;
}  // namespace

IpMatch IpMatch::parse( const std::string& text )
{
  const auto delim = text.find_first_of( '/' );
  if ( delim == std::string::npos )
    return IpMatch( inet_addr( text.c_str() ), exact_mask );

  return IpMatch( inet_addr( text.substr( 0, delim ).c_str() ),
                  inet_addr( text.substr( delim + 1 ).c_str() ) );
}

bool IpMatch::matches( const sockaddr& peer ) const
{
  if ( peer.sa_family != AF_INET )
    return false;

  const auto& in = reinterpret_cast<const sockaddr_in&>( peer );
  return ( _addr & _mask ) == ( in.sin_addr.s_addr & _mask );
}

bool matches_any( const std::vector<IpMatch>& filters, const sockaddr& peer )
{
  return std::any_of( filters.begin(), filters.end(),
                      [&peer]( const IpMatch& filter ) { return filter.matches( peer ); } );
}

void read_ip_match_list( Clib::ConfigElem& elem, const char* tag, std::vector<IpMatch>* filters )
{
  std::string text;
  while ( elem.remove_prop( tag, &text ) )
    filters->push_back( IpMatch::parse( text ) );
}
}  // namespace Pol::Network
