#include <algorithm>
#include <string>
#include <utility>

#include "clib/rawtypes.h"
#include "pol/testing/testenv.h"

namespace Pol::Mobile
{
// Defined in charactr.cpp with no header of its own; npc.cpp declares it the same way.
unsigned short calc_thru_damage( double damage, unsigned short ar );
}  // namespace Pol::Mobile

namespace Pol::Testing
{
namespace
{
// The lowest and highest damage calc_thru_damage returned over many draws. Armor absorbs half
// its value plus a random part of the rest, so the result is a range, not a value.
std::pair<unsigned short, unsigned short> thru_damage_range( double damage, unsigned short ar )
{
  unsigned short lo = 0xFFFF;
  unsigned short hi = 0;
  for ( int i = 0; i < 500; ++i )
  {
    unsigned short d = Mobile::calc_thru_damage( damage, ar );
    lo = std::min( lo, d );
    hi = std::max( hi, d );
  }
  return { lo, hi };
}
}  // namespace

void combat_test()
{
  // Without armor nothing is random: damage of 2 or more is halved, less is kept as it is.
  UnitTest( [] { return Mobile::calc_thru_damage( 100.0, 0 ); }, u16( 50 ),
            "calc_thru_damage halves damage when there is no armor" );
  UnitTest( [] { return Mobile::calc_thru_damage( 1.5, 0 ); }, u16( 1 ),
            "calc_thru_damage keeps damage below 2 unhalved" );
  UnitTest( [] { return Mobile::calc_thru_damage( 200000.0, 0 ); }, u16( 0xFFFF ),
            "calc_thru_damage clamps to the largest u16" );

  // 20 armor absorbs 10 to 20 points of 100, and what is left is halved: 40 to 45.
  UnitTest(
      []
      {
        auto [lo, hi] = thru_damage_range( 100.0, 20 );
        return lo >= 40 && hi <= 45 && lo < hi;
      },
      true, "calc_thru_damage with 20 armor stays within 40..45 and varies" );

  // Armor worth more than the damage absorbs all of it, and a negative remainder is 0, not a
  // wrapped u16.
  UnitTest(
      []
      {
        auto [lo, hi] = thru_damage_range( 10.0, 40 );
        return lo == 0 && hi == 0;
      },
      true, "calc_thru_damage with armor above the damage is 0" );
}
}  // namespace Pol::Testing
