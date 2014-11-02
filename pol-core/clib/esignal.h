/*
History
=======

Notes
=======

*/

#include <atomic>

namespace Pol {
  namespace Clib {
	extern std::atomic<bool> exit_signalled;

	void enable_exit_signaller();
  }
}