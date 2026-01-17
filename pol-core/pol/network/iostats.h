/** @file
 *
 * @par History
 */

#ifndef __IOSTATS_H
#define __IOSTATS_H

#include <atomic>

namespace Pol::Network
{
class IOStats
{
public:
  IOStats();

  struct Packet
  {
    std::atomic<unsigned int> count;
    std::atomic<unsigned int> bytes;
  };

  Packet sent[256];
  Packet received[256];
};
}  // namespace Pol::Network

#endif
