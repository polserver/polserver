/** @file
 *
 * @par History
 */


// TODO: REWORK ME!!
//
//#include "../pol/uconst.h"
//#include "../clib/logfacility.h"
//#include "realm.h"
//#include "mapserver.h"
//
//namespace Pol {
//  namespace Plib {
//  void inc_successes();
//  void inc_failures();
//
//  extern Realm* p_test_realm;
//
//  // This code should mirror code in drop_item_on_ground
//  static void test_drop( unsigned short chrx, unsigned short chry, short chrz,
//               unsigned short dropx, unsigned short dropy, short dropz, bool exp_result, short exp_z )
//  {
//    if ( p_test_realm == NULL )
//    p_test_realm = new Realm( "britannia" );
//
//      fmt::Writer tmp;
//      tmp << "POL DropHeight(" << chrx << "," << chry << "," << chrz << ","
//    << dropx << "," << dropy << "," << dropz << "): "
//    << "Expect " << exp_result << "," << exp_z << ": ";
//
//    short newz;
//    Multi::UMulti* multi;
//    bool result = p_test_realm->dropheight( dropx, dropy, dropz, chrz, &newz, &multi );
//      tmp << "Got " << result << "," << newz << ": ";
//    if ( exp_result != result )
//    {
//        tmp << "Failure!\n";
//        INFO_PRINT << tmp.str();
//    inc_failures();
//    return;
//    }
//    if ( result )
//    {
//    if ( newz != exp_z )
//    {
//          tmp << "Failure!\n";
//          INFO_PRINT << tmp.str();
//      inc_failures();
//      return;
//    }
//    }
//    inc_successes();
//      INFO_PRINT << tmp.str() << "Ok!\n";
//  }
//
//  void pol_drop_test( void )
//  {
//      INFO_PRINT << "POL datafile drop tests:\n";
//    // first things first.  Gotta be able to drop stuff by the brit bank.
//    test_drop( 1432, 1696, 0, 1433, 1696, 0, true, 0 );
//    // in the bank, on the floor
//    test_drop( 1437, 1687, 0, 1437, 1688, 0, true, 0 );
//
//    // on a desk in LB's castle
//    test_drop( 1328, 1645, 72, 1328, 1644, 78, true, 78 );
//
//    // one of these screwy undermap caves
//    test_drop( 2564, 489, 0, 2564, 489, 0, true, 0 );
//    test_drop( 2564, 489, 42, 2564, 489, 42, false, 0 );
//
//    // a liche room
//    test_drop( 5310, 735, 0, 5310, 735, 0, true, 0 );
//
//    // a downward slope
//    test_drop( 5304, 2488, 39, 5304, 2489, 35, true, 35 );
//
//    // a ramp in an orc fort
//    test_drop( 5210, 3616, 0, 5210, 3615, 5, true, 2 );
//  }
//  }
//}
//

