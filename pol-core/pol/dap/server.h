#ifndef DAP_SERVER_H
#define DAP_SERVER_H

#include <memory>


namespace dap::net
{
class Server;
}


namespace Pol::Network::DAP
{
class DebugServer
{
public:
  DebugServer();
  ~DebugServer();

private:
  std::unique_ptr<dap::net::Server> _server;
};
}  // namespace Pol::Network::DAP


#endif
