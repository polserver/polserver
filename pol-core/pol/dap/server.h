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
namespace DAP
{


class DebugServer
{
public:
  DebugServer();
  ~DebugServer();

private:
  std::unique_ptr<dap::net::Server> _server;
};
}  // namespace DAP
}  // namespace Network
}  // namespace Pol
#endif
