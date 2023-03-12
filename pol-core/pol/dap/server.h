#ifndef POLDBGDAP_H
#define POLDBGDAP_H

#include <dap/network.h>

namespace Pol
{
namespace Network
{
class DapDebugServer
{
public:
  DapDebugServer();

private:
  std::unique_ptr<dap::net::Server> _server;
};
}  // namespace Network
}  // namespace Pol
#endif
