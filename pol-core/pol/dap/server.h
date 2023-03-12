#ifndef POLDBGDAP_H
#define POLDBGDAP_H

#include <memory>

namespace dap
{
namespace net
{
class Server;
}
}  // namespace dap

namespace Pol
{
namespace Network
{
class DapDebugServer
{
public:
  DapDebugServer();
  ~DapDebugServer();

private:
  std::unique_ptr<dap::net::Server> _server;
};
}  // namespace Network
}  // namespace Pol
#endif
