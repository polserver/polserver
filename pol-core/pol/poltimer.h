/** @file
 *
 * @par History
 */


#ifndef POLTIMER_H
#define POLTIMER_H

#include <iosfwd>
#include "polclock.h"
namespace Pol {
  namespace Core {
	class PolTimer
	{
	public:
	  PolTimer() : start( polclock() ) {}
	  void printOn( std::ostream& os ) { os << "elapsed: " << polclock() - start << " polclocks\n"; }

	private:
	  polclock_t start;
	};
  }
}
#endif
