#include "server.h"

#include "clientthread.h"

#include "../../clib/logfacility.h"
#include "../../clib/threadhelp.h"
#include "../../plib/systemstate.h"
#include "../globals/network.h"

#include <dap/network.h>

namespace Pol
{
namespace Network
{
namespace DAP
{
DebugServer::DebugServer()
{
  _server = dap::net::Server::create();

  // If DebugLocalOnly, bind to localhost which allows connections only from local addresses.
  // Otherwise, bind to any address to also allow remote connections.
  auto address = Plib::systemstate.config.debug_local_only ? "localhost" : "0.0.0.0";

  auto started =
      _server->start( address, Plib::systemstate.config.dap_debug_port,
                      []( const std::shared_ptr<dap::ReaderWriter>& rw )
                      {
                        auto client = std::make_shared<DebugClientThread>( rw );
                        Core::networkManager.auxthreadpool->push( [=]() { client->run(); } );
                      } );

  if ( !started )
  {
    POLLOG_ERRORLN( "Failed to start DAP server." );
    _server.reset();
  }
}

DebugServer::~DebugServer() = default;
}  // namespace DAP
}  // namespace Network
}  // namespace Pol