/** @file
 *
 * @par History
 */


#ifndef POLTEST_H
#define POLTEST_H

#define ENABLE_POLTEST_OUTPUT 0
namespace Pol {
  namespace Core {
	void run_pol_tests();
	void inc_failures();
	void inc_successes();
  }
}
#endif
