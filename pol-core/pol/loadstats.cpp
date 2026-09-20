/** @file
 *
 * @par History
 */


#include "pol/loadstats.h"

#include <fmt/format.h>

#include "clib/logfacility.h"

namespace Pol::Core
{
WorldLoadStats worldLoadStats;

void WorldLoadStats::reset()
{
  parse_ns = 0;
  create_ns = 0;
  hash_ns = 0;
  props_ns = 0;
  cprops_ns = 0;
  place_ns = 0;
}

void WorldLoadStats::log( const std::string& what, u64 elements ) const
{
  if ( !enabled || elements == 0 )
    return;

  // Nested phases are reported net of what they contain, so the column sums to the file's time
  // rather than counting the inner ones twice.
  const u64 create_own = create_ns > hash_ns ? create_ns - hash_ns : 0;
  const u64 props_own = props_ns > cprops_ns ? props_ns - cprops_ns : 0;
  const u64 accounted = parse_ns + create_ns + props_ns + place_ns;

  fmt::memory_buffer buffer;
  auto out = std::back_inserter( buffer );
  fmt::format_to( out, "  {} breakdown over {} elements:", what, elements );

  // us/element alongside the total: it separates a file that is slow because it is big from one
  // whose elements are each dear.
  auto line = [&]( const char* name, u64 ns )
  {
    fmt::format_to( out, "\n    {:<22} {:>7} ms {:>8.2f} us/element", name, ns / 1000000,
                    static_cast<double>( ns ) / 1000.0 / static_cast<double>( elements ) );
  };
  line( "parse", parse_ns );
  line( "create", create_own );
  line( "  of it object hash", hash_ns );
  line( "readProperties", props_own );
  line( "  of it cprops", cprops_ns );
  line( "place", place_ns );
  line( "accounted for", accounted );

  POLLOG_INFOLN( "{}", std::string_view( buffer.data(), buffer.size() ) );
}
}  // namespace Pol::Core
