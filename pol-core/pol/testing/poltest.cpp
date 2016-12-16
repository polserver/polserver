/** @file
 *
 * @par History
 */

#include "poltest.h"
#include "testenv.h"

namespace Pol
{
namespace Testing
{

void run_pol_tests()
{
  create_test_environment();
  
  skilladv_test();

  drop_test();
  walk_test();
  los_test();
  multiwalk_test();
  map_test();
  dynprops_test();

  display_test_results();
}

}
}
