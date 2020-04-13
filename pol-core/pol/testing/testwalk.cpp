/** @file
 *
 * @par History
 */

#include "../../clib/logfacility.h"
#include "../../plib/uconst.h"
#include "../globals/uvars.h"
#include "../realms/realm.h"
#include "testenv.h"

namespace Pol
{
namespace Testing
{
namespace
{
using namespace Items;
using namespace Multi;
using namespace Core;

void test_walk( unsigned short /*oldx*/, unsigned short /*oldy*/, short oldz, unsigned short newx,
                unsigned short newy, bool exp_result, short exp_z )
{
  short newz;

  INFO_PRINT << "WalkHeight(" << newx << "," << newy << "," << oldz << "): "
             << "Expect " << exp_result << "," << exp_z << ": ";
  UMulti* multi;
  Item* itm;
  bool res = gamestate.main_realm->walkheight( Core::Pos2d( newx, newy ), oldz, &newz, &multi, &itm,
                                               true /*doors
block*/, Plib::MOVEMODE_LAND );

  INFO_PRINT << "Got " << res << "," << newz << ": ";

  if ( exp_result != res )
  {
    INFO_PRINT << "Failure!\n";
    inc_failures();
    return;
  }
  if ( exp_result )  // Z is only defined if result is true
  {
    if ( exp_z != newz )
    {
      INFO_PRINT << "Failure!\n";
      inc_failures();
      return;
    }
  }
  inc_successes();
  INFO_PRINT << "Ok!\n";
}

void test_walk2( unsigned short /*oldx*/, unsigned short /*oldy*/, short oldz, unsigned short newx,
                 unsigned short newy, bool doors_block, Plib::MOVEMODE movemode, bool exp_result,
                 short exp_z )
{
  short newz;

  INFO_PRINT << "WalkHeight(" << newx << "," << newy << "," << oldz << "): "
             << "Expect " << exp_result << "," << exp_z << ": ";
  UMulti* multi;
  Item* itm;
  bool res = gamestate.main_realm->walkheight( Core::Pos2d( newx, newy ), oldz, &newz, &multi, &itm,
                                               doors_block, movemode );

  INFO_PRINT << "Got " << res << "," << newz << ": ";

  if ( exp_result != res )
  {
    INFO_PRINT << "Failure!\n";
    inc_failures();
    return;
  }
  if ( exp_result )  // Z is only defined if result is true
  {
    if ( exp_z != newz )
    {
      INFO_PRINT << "Failure!\n";
      inc_failures();
      return;
    }
  }
  inc_successes();
  INFO_PRINT << "Ok!\n";
}
}  // namespace

void walk_test()
{
  INFO_PRINT << "POL datafile tests:\n";
  test_walk( 1344, 1637, 72, 1345, 1637, true, 72 );

  // walk down some stairs in Castle Britannia
  test_walk( 1352, 1635, 72, 1352, 1634, true, 72 );
  test_walk( 1352, 1634, 72, 1352, 1633, true, 72 );
  test_walk( 1352, 1633, 72, 1352, 1632, true, 69 );
  test_walk( 1352, 1632, 69, 1352, 1631, true, 64 );
  test_walk( 1352, 1631, 64, 1352, 1630, true, 59 );
  test_walk( 1352, 1630, 59, 1352, 1629, true, 54 );
  test_walk( 1352, 1629, 54, 1352, 1628, true, 50 );
  test_walk( 1352, 1628, 50, 1352, 1627, true, 50 );

  test_walk( 1381, 1625, 30, 1382, 1625, true, 30 );
  test_walk( 1382, 1626, 30, 1383, 1625, true, 30 );

  test_walk( 1386, 1628, 30, 1386, 1629, true, 30 );

  // on the bridge
  test_walk( 1402, 1626, 28, 1402, 1625, true, 28 );
  // leaving the bridge
  test_walk( 1408, 1625, 28, 1409, 1625, true, 28 );
  test_walk( 1409, 1625, 28, 1410, 1625, true, 28 );
  test_walk( 1410, 1625, 28, 1411, 1625, true, 26 );
  test_walk( 1411, 1625, 26, 1412, 1625, true, 25 );
  test_walk( 1412, 1625, 25, 1413, 1625, true, 24 );
  test_walk( 1413, 1625, 25, 1414, 1625, true, 23 );
  test_walk( 1414, 1625, 25, 1415, 1625, true, 22 );
  test_walk( 1415, 1625, 25, 1416, 1625, true, 21 );
  test_walk( 1416, 1625, 25, 1417, 1625, true, 20 );
  test_walk( 1417, 1625, 25, 1418, 1625, true, 20 );
  test_walk( 1418, 1625, 25, 1419, 1625, true, 20 );

  // static_debug_on = true;
  test_walk( 1377, 1759, 1, 1378, 1759, true, 1 );

  // try walking through a static wall
  test_walk( 1345, 1649, 72, 1346, 1649, false, 0 );

  // test the walking on a floor, with a dynamic below you, screwing up.
  test_walk( 1347, 1659, 72, 1347, 1658, true, 72 );
  test_walk( 1347, 1658, 72, 1347, 1657, true, 72 );


  // back on the bridge
  // static_debug_on = true;
  test_walk( 1398, 1624, 29, 1399, 1624, true, 28 );

  // try walking through a portcullis
  test_walk( 1399, 1625, 27, 1398, 1625, false, 0 );

  // try walking onto a stone step
  test_walk( 1310, 1630, 30, 1309, 1630, true, 31 );

  // try walking onto a dungeon ramp
  test_walk( 1311, 1637, 30, 1311, 1636, true, 32 );

  // try walking under a stone arch
  test_walk( 1311, 1650, 30, 1311, 1649, true, 30 );

  // try walking through a secret door
  test_walk( 1316, 1649, 30, 1316, 1648, true, 30 );

  // try walking through a hide-type door
  test_walk( 1314, 1654, 30, 1313, 1654, true, 30 );

  // try walking under a dungeon arch
  test_walk( 1313, 1658, 30, 1313, 1657, true, 30 );

  // try walking over a black pearl
  test_walk( 1315, 1657, 30, 1315, 1658, true, 30 );


  // walk down some stairs, um, somewhere
  test_walk( 5920, 40, 44, 5920, 41, true, 41 );
  test_walk( 5920, 41, 41, 5920, 42, true, 36 );
  test_walk( 5920, 42, 36, 5920, 43, true, 31 );
  test_walk( 5920, 43, 31, 5920, 44, true, 26 );
  test_walk( 5920, 44, 26, 5920, 45, true, 22 );
  test_walk( 5920, 45, 22, 5921, 45, true, 22 );
  test_walk( 5921, 45, 22, 5922, 45, true, 22 );
  // walk up those same stairs
  test_walk( 5920, 45, 22, 5920, 44, true, 26 );
  test_walk( 5920, 44, 26, 5920, 43, true, 31 );
  test_walk( 5920, 43, 31, 5920, 42, true, 36 );
  test_walk( 5920, 42, 36, 5920, 41, true, 41 );
  test_walk( 5920, 41, 41, 5920, 40, true, 44 );
  test_walk( 5920, 40, 44, 5920, 39, true, 44 );
  test_walk( 5921, 40, 44, 5922, 40, true, 44 );

  // static_debug_on = true;
  // walking onto a table?
  test_walk( 3664, 2589, 0, 3664, 2588, false, 0 );
  // walking onto a table
  test_walk( 1594, 1588, 20, 1594, 1587, false, 0 );

  test_walk( 1262, 1246, 0, 1262, 1245, true, 0 );
  test_walk( 1264, 1253, 0, 1264, 1252, true, 0 );

  test_walk( 1441, 1660, 9, 1442, 1660, true, 10 /*9*/ );

  // near the britain bank,a steep walk
  test_walk( 1426, 1703, 3, 1425, 1704, true, 9 );

  // walk up into an orc fort
  test_walk( 2163, 1330, 0, 2163, 1329, true, 7 );

  // walk through the gates of Cove
  test_walk( 2284, 1209, 0, 2283, 1209, true, 0 );

  // weird stairs in a dungeon
  test_walk( 5177, 188, 10, 5177, 187, true, 20 );

  // wind is pissinhg me off...
  test_walk( 5166, 179, 17, 5166, 180, true, 25 );

  // a steep grade in ocllo
  test_walk( 3773, 2654, 0, 3772, 2654, true, 14 );

  // walk onto a teleporter
  test_walk( 1175, 2636, 0, 1175, 2635, true, 0 );

  // walking on a peninsula
  test_walk( 1909, 51, 0, 1909, 50, true, 0 );
  test_walk( 1909, 50, 0, 1910, 50, true, -2 );
  test_walk( 1909, 50, 0, 1909, 49, true, -1 );
  test_walk( 1909, 50, 0, 1908, 49, true, -2 );
  test_walk( 1908, 49, -2, 1907, 49, true, -3 );

  // This is known to fail - diagonal movement here should fail, but is allowed.
  test_walk( 1908, 51, 0, 1907, 50, false, 0 );

  test_walk( 1908, 51, 0, 1908, 50, true, 0 );
  test_walk( 1908, 50, 0, 1907, 50, true, -1 );
  test_walk( 1909, 51, 0, 1908, 50, true, 0 );
  test_walk( 1908, 50, 0, 1907, 49, true, -3 );
  test_walk( 1907, 49, -3, 1906, 49, true, -4 );

  // ladders
  test_walk( 2069, 817, 20, 2068, 817, true, 26 );
  test_walk( 2068, 817, 26, 2067, 817, true, 37 );
  test_walk( 2043, 807, 20, 2042, 807, true, 26 );
  test_walk( 2042, 807, 26, 2041, 807, true, 37 );

  // stairs in T2A
  test_walk( 2675, 2280, -40, 2675, 2279, true, -35 );
  test_walk( 2675, 2279, -35, 2675, 2278, true, -29 );
  // this one asks to be able to step up 9 squares
  test_walk( 2675, 2278, -29, 2675, 2277, true, -20 );

  // Ladder by Yew
  test_walk( 968, 709, -40, 967, 709, true, -35 );
  test_walk( 967, 709, -35, 966, 709, true, -24 );
  test_walk( 966, 709, -24, 965, 709, true, -20 );

  // walking on water
  test_walk2( 481, 306, -5, 481, 307, true, Plib::MOVEMODE_SEA, true, -5 );
  test_walk2( 481, 306, -5, 481, 307, true, Plib::MOVEMODE_LAND, false, 0 );

  // try walking through a fence
  test_walk( 1377, 1492, 10, 1377, 1493, false, 0 );

  // check a static roof tile (shouldn't be able to walk there)
  test_walk( 1508, 1551, 45, 1508, 1552, false, 0 );

  // some odd brambles that shouldn't block movement.
  test_walk( 1058, 1449, 0, 1059, 1449, true, 0 );


  // an odd cave
  test_walk( 2605, 458, 60, 2605, 457, true, 60 );
  test_walk( 2605, 457, 60, 2605, 456, true, 60 );
  test_walk( 2605, 456, 62, 2605, 455, true, 60 );
  test_walk( 4723, 3814, 0, 4723, 3813, true, 0 );
  test_walk( 4110, 431, 5, 4110, 430, true, 5 );
  test_walk( 1661, 2938, 0, 1661, 2937, true, 0 );
  test_walk( 1652, 2893, 0, 1651, 2893, true, 0 );

  // a secret door through the ground:
  test_walk( 1146, 2228, 20, 1145, 2228, true, 20 );
  test_walk( 1145, 2228, 20, 1144, 2228, true, 20 );
  test_walk( 1144, 2228, 20, 1143, 2228, true, 20 );
  test_walk( 1143, 2228, 20, 1142, 2228, true, 20 );
  test_walk( 1142, 2228, 20, 1141, 2228, true, 20 );
}

void multiwalk_test()
{
  INFO_PRINT << "POL datafile multi walk tests:\n";
  // now we have a small house in the hills
  test_walk( 2016, 1332, 0, 2016, 1331, true, 4 );
  test_walk( 2016, 1331, 4, 2017, 1331, true, 4 );
  test_walk( 2017, 1331, 4, 2018, 1331, true, 0 );
  test_walk( 2018, 1331, 0, 2018, 1330, false, 0 );  // walk N, blocked
  test_walk( 2018, 1331, 0, 2019, 1331, true, 0 );
  test_walk( 2019, 1331, 0, 2019, 1330, false, 0 );
  test_walk( 2019, 1331, 0, 2020, 1331, true, 0 );
  test_walk( 2020, 1331, 0, 2020, 1330, true, 0 );
  test_walk( 2016, 1331, 4, 2016, 1330, true, 7 );
  test_walk( 2016, 1330, 7, 2015, 1330, false, 0 );
  test_walk( 2016, 1330, 7, 2017, 1330, false, 0 );


  // try walking around on a boat
  test_walk( 1487, 1770, -2, 1487, 1769, true, -2 );
  test_walk( 1487, 1769, -2, 1487, 1768, false, 0 );  // try to walk into the mast
  test_walk( 1487, 1769, -2, 1486, 1769, true, -2 );
  test_walk( 1486, 1769, -2, 1486, 1768, true, -2 );
  // this commented out because the planks are both retracted.
  // test_walk( 1486, 1768, -2,      1485, 1768,     true, -4 ); // on the gangplank
  test_walk( 1485, 1768, -4, 1485, 1769, false, 0 );  // try to walk into the water
  test_walk( 1485, 1768, -4, 1485, 1767, false, 0 );  // try to walk into the water
  test_walk( 1485, 1768, -4, 1484, 1768, true, -2 );
  test_walk( 1487, 1765, -2, 1487, 1764, true, -2 );  // try to walk onto the hold

  // try walking around on a boat in deeper water
  test_walk( 1497, 1781, -2, 1497, 1780, true, -2 );
  test_walk( 1497, 1780, -2, 1497, 1779, false, 0 );  // try to walk into the mast
  test_walk( 1497, 1780, -2, 1496, 1780, true, -2 );
  test_walk( 1496, 1780, -2, 1496, 1779, true, -2 );
  test_walk( 1496, 1779, -2, 1495, 1779, true, -4 );  // on the gangplank
  test_walk( 1495, 1779, -4, 1495, 1778, false, 0 );  // try to walk into the water
  test_walk( 1495, 1779, -4, 1495, 1780, false, 0 );  // try to walk into the water
  test_walk( 1495, 1779, -4, 1494, 1779, false, 0 );

  // try walking on a long boat, next to its plank
  test_walk( 1496, 1817, -2, 1495, 1817, true, -2 );
}
}  // namespace Testing
}  // namespace Pol
