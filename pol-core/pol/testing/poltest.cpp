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

void run_pol_tests()
{
  create_test_environment();

#ifdef ENABLE_BENCHMARK
  benchmark::RunSpecifiedBenchmarks();
  return;
#endif

//  skilladv_test();

//  drop_test();
//  walk_test();
//  multiwalk_test();
//  map_test();
//  dynprops_test();
  packet_test();
  dummy();
  display_test_results();
}

}
}
