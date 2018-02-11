/** @file
 *
 * I doubt the usefullness..
 * raw_to_base looses information
 * @par History
 */


#include "testenv.h"

#include "../../clib/logfacility.h"
#include "../skilladv.h"

namespace Pol
{
namespace Testing
{
namespace
{
void test_skilladv( unsigned int raw, unsigned short base )
{
  INFO_PRINT << "Raw " << raw << ", base " << base << ": ";
  if ( Core::raw_to_base( raw ) != base )
  {
    INFO_PRINT << "raw_to_base failed!\n";
    inc_failures();
  }
  if ( Core::base_to_raw( base ) != raw )
  {
    INFO_PRINT << "base_to_raw failed!\n";
    inc_failures();
  }
  INFO_PRINT << "Passed\n";
  inc_successes();
}
}  // namespace

void skilladv_test()
{
  INFO_PRINT << "Skill advancement tests:\n";
  test_skilladv( 20, 1 );
  test_skilladv( 512, 25 );
  test_skilladv( 1024, 50 );
  test_skilladv( 2048, 100 );
  test_skilladv( 3072, 150 );
  test_skilladv( 4096, 200 );
  test_skilladv( 5120, 225 );
  test_skilladv( 6144, 250 );
  test_skilladv( 7168, 275 );
  test_skilladv( 8192, 300 );
  test_skilladv( 16384, 400 );
  test_skilladv( 32768, 500 );
  test_skilladv( 65536, 600 );
  test_skilladv( 131072, 700 );
  test_skilladv( 262144, 800 );
  test_skilladv( 524288, 900 );
  test_skilladv( 1048576, 1000 );
  test_skilladv( 2097152, 1100 );
  test_skilladv( 4194304, 1200 );
  test_skilladv( 8388608, 1300 );
  test_skilladv( 16777216, 1400 );
  test_skilladv( 33554432, 1500 );
  test_skilladv( 67108864, 1600 );
  test_skilladv( 134217728, 1700 );
  test_skilladv( 268435456, 1800 );
  test_skilladv( 536870912, 1900 );

  for ( unsigned short base = 0; base < 2000; ++base )
  {
    INFO_PRINT << "Base " << base << ": Raw=";
    unsigned int raw = Core::base_to_raw( base );
    INFO_PRINT << raw << ": ";
    unsigned short calcbase = Core::raw_to_base( raw );
    if ( calcbase != base )
    {
      INFO_PRINT << "Failed (" << calcbase << ")\n";
      inc_failures();
    }
    else
    {
      INFO_PRINT << "Passed\n";
      inc_successes();
    }
  }

  for ( unsigned int raw = 0; raw < 2500; ++raw )
  {
    INFO_PRINT << "Raw " << raw << ": Base=";
    unsigned short base = Core::raw_to_base( raw );
    INFO_PRINT << base << ": ";
    unsigned int calcraw = Core::base_to_raw( base );
    if ( calcraw != raw )
    {
      INFO_PRINT << "Failed (" << calcraw << ")\n";
      inc_failures();
    }
    else
    {
      INFO_PRINT << "Passed\n";
      inc_successes();
    }
  }
}

}  // namespace Testing
}  // namespace Pol
