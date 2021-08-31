/** @file
 *
 * @par History
 */

#include "poltest.h"

#include "testenv.h"

#include "pol_global_config.h"

#ifdef ENABLE_BENCHMARK
#include <benchmark/benchmark.h>
#endif

namespace Pol
{
namespace Testing
{
#define RUNTEST( t )                     \
  INFO_PRINT << " - " << ( #t ) << "\n"; \
  t();

bool run_pol_tests()
{
  // create_test_environment();

#ifdef ENABLE_BENCHMARK
  benchmark::RunSpecifiedBenchmarks();
  return;
#endif
  RUNTEST( test_splitnamevalue )
  RUNTEST( test_convertquotedstring )

  //  skilladv_test();

  //  drop_test();
  //  walk_test();
  //  multiwalk_test();
  //  map_test();
  RUNTEST( dynprops_test )
  RUNTEST( packet_test )
  RUNTEST( vector2d_test )
  RUNTEST( vector3d_test )
  RUNTEST( pos2d_test )
  RUNTEST( pos3d_test )
//  RUNTEST( dummy )

  UnitTest::display_test_results();
  return UnitTest::result();
}

}  // namespace Testing
}  // namespace Pol
