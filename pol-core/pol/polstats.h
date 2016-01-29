/** @file
 *
 * @par History
 */


#ifndef POLSTATS_H
#define POLSTATS_H

#include "../clib/rawtypes.h"
#include <atomic>
namespace Pol {
  namespace Core {
	class PolStats
	{
	public:
      PolStats();
	  std::atomic<u64> bytes_received;
	  std::atomic<u64> bytes_sent;
	};
	//extern PolStats auxstats; (Not yet... -- Nando)
	//extern PolStats webstats;
  }
}
#endif
