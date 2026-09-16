/** @file
 *
 * @par History
 */

#include <string>
#include <utility>
#include <vector>

#include "clib/rawtypes.h"
#include "pol/testing/testenv.h"
#include "pol/textcmd.h"


namespace Pol::Testing
{
namespace
{
// The colour slot of an equip.cfg line, rendered as "low-high[,low-high...]" in hex, with a
// trailing "!" when an entry was dropped as unreadable.
std::string parsed( const std::string& spec )
{
  bool malformed = false;
  auto choices = Core::parse_equip_colors( spec, &malformed );
  std::string out;
  for ( const auto& range : choices )
  {
    if ( !out.empty() )
      out += ',';
    out += fmt::format( "{:#x}-{:#x}", range.first, range.second );
  }
  if ( malformed )
    out += '!';
  return out;
}

void check( const std::string& spec, const std::string& expected )
{
  UnitTest( [&]() { return parsed( spec ); }, expected,
            fmt::format( " '{}' -> '{}'", spec, expected ) );
}
}  // namespace

// What EquipFromTemplate accepts in the colour slot: a value, an inclusive low-high range, or a
// comma separated list of either. A shard's equip.cfg is hand written, so leniency matters as much
// as the grammar: a reversed range is kept (the pick swaps it), text after a value is ignored the
// way the one-colour form always ignored it, and one bad entry costs only itself.
void equip_colors_test()
{
  // no colour at all
  check( "", "" );
  check( "   ", "" );

  // single value, decimal or hex, as before
  check( "1701", "0x6a5-0x6a5" );
  check( "0x461", "0x461-0x461" );
  check( "0x21 whatever", "0x21-0x21" );

  // range
  check( "1701-1754", "0x6a5-0x6da" );
  check( "1150 - 1154", "0x47e-0x482" );
  check( "0x461-0x470", "0x461-0x470" );
  check( "1754-1701", "0x6da-0x6a5" );

  // list, with and without a range in it
  check( "1940, 1965, 1109", "0x794-0x794,0x7ad-0x7ad,0x455-0x455" );
  check( "1940 ,1965", "0x794-0x794,0x7ad-0x7ad" );
  check( "1940, 1965, 1109, 1150-1154", "0x794-0x794,0x7ad-0x7ad,0x455-0x455,0x47e-0x482" );

  // a value wider than a colour is clamped rather than wrapped
  check( "0xffffff", "0xffff-0xffff" );

  // unreadable entries are dropped, whatever else is on the line
  check( "1701-", "!" );
  check( "-5", "!" );
  check( "abc", "!" );
  check( "1940,,1965", "0x794-0x794,0x7ad-0x7ad!" );
  check( "1940, abc", "0x794-0x794!" );
}

}  // namespace Pol::Testing
