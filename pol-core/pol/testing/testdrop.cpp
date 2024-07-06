/** @file
 *
 * @par History
 */

#include "../../clib/logfacility.h"
#include "../globals/uvars.h"
#include "../realms/realm.h"
#include "testenv.h"
#include <fmt/format.h>

namespace Pol
{
namespace Testing
{
namespace
{
// This code should mirror code in drop_item_on_ground
void test_drop( unsigned short chrx, unsigned short chry, short chrz, unsigned short dropx,
                unsigned short dropy, short dropz, bool exp_result, short exp_z )
{
  std::string tmp = fmt::format( "POL DropHeight({},{},{},{},{},{}): Expect {}<{}: ", chrx, chry,
                                 chrz, dropx, dropy, dropz, exp_result, exp_z );

  short newz;
  Multi::UMulti* multi;
  bool result = Core::gamestate.main_realm->dropheight( Core::Pos3d( dropx, dropy, (s8)dropz ),
                                                        chrz, &newz, &multi );
  tmp += fmt::format( "Got {},{}: ", result, newz );
  if ( exp_result != result )
  {
    tmp += "Failure!";
    INFO_PRINTLN( tmp );
    UnitTest::inc_failures();
    return;
  }
  if ( result )
  {
    if ( newz != exp_z )
    {
      tmp += "Failure!";
      INFO_PRINTLN( tmp );
      UnitTest::inc_failures();
      return;
    }
  }
  UnitTest::inc_successes();
  INFO_PRINTLN( "{}Ok!", tmp );
}
}  // namespace

void drop_test()
{
  INFO_PRINTLN( "POL datafile drop tests:" );
  // first things first.  Gotta be able to drop stuff by the brit bank.
  test_drop( 1432, 1696, 0, 1433, 1696, 0, true, 0 );
  // in the bank, on the floor
  test_drop( 1437, 1687, 0, 1437, 1688, 0, true, 0 );

  // on a desk in LB's castle
  test_drop( 1328, 1645, 72, 1328, 1644, 78, true, 78 );

  // one of these screwy undermap caves
  test_drop( 2564, 489, 0, 2564, 489, 0, true, 0 );
  test_drop( 2564, 489, 42, 2564, 489, 42, false, 0 );

  // a liche room
  test_drop( 5310, 735, 0, 5310, 735, 0, true, 0 );

  // a downward slope
  test_drop( 5304, 2488, 39, 5304, 2489, 35, true, 35 );

  // a ramp in an orc fort
  test_drop( 5210, 3616, 0, 5210, 3615, 5, true, 2 );
}
}  // namespace Testing
}  // namespace Pol
