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
bool run_pol_tests()
{
  // create_test_environment();

#ifdef ENABLE_BENCHMARK
  benchmark::RunSpecifiedBenchmarks();
  return;
#endif
  test_splitnamevalue();
  test_convertquotedstring();

  //  skilladv_test();

  //  drop_test();
  //  walk_test();
  //  multiwalk_test();
  //  map_test();
  dynprops_test();
  packet_test();
  dummy();

  UnitTest::display_test_results();
  return UnitTest::result();
}

}  // namespace Testing
}  // namespace Pol
