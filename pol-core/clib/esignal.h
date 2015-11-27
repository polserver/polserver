/*
History
=======

Notes
=======

*/
#ifndef CLIB_ESIGNAL_H
#define CLIB_ESIGNAL_H

#include <atomic>

namespace Pol {
  namespace Clib {
	extern std::atomic<bool> exit_signalled;

	void enable_exit_signaller();
  }
}

#endif //CLIB_ESIGNAL_H