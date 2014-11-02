/*
History
=======


Notes
=======

*/

#include "iostats.h"

#include <string.h>

namespace Pol {
  namespace Network {
	IOStats::IOStats()
	{
	  memset( &sent, 0, sizeof sent );
	  memset( &received, 0, sizeof received );
	}

	IOStats iostats;
	IOStats queuedmode_iostats;
  }
}