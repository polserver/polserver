/*
History
=======


Notes
=======

*/
#ifndef __DECAY_H
#define __DECAY_H

#include <cstddef>

namespace Pol {
  namespace Core {
	void decay_items();
	void decay_single_zone();

	extern size_t cycles_per_decay_worldzone;
	extern size_t cycles_until_decay_worldzone;
  }
}
#endif
