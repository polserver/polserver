#ifndef POLDBGDAP_H
#define POLDBGDAP_H

namespace Pol
{
namespace Network
{
class DapDebugServer
{
public:
  static void initialize();
  static void deinitialize();
};
}  // namespace Network
}  // namespace Pol
#endif
