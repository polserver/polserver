#include "server.h"

#include "../../clib/esignal.h"
#include "../../clib/logfacility.h"
#include "../../clib/threadhelp.h"
#include "../../plib/systemstate.h"
#include "../globals/network.h"
#include "../polclock.h"
#include "../polsem.h"

#include <dap/io.h>
#include <dap/protocol.h>
#include <dap/session.h>
#include <memory>

namespace Pol
{
namespace Core
{
// Implements the dap::ReaderWriter IO interface using a Clib::Socket
class SocketReaderWriter : public dap::ReaderWriter
{
public:
  SocketReaderWriter( Clib::Socket&& sck ) : _sck( std::move( sck ) ) {}

  size_t read( void* buffer, size_t n ) override
  {
    int bytes_read = 0;

    if ( !_sck.recvdata_nowait( static_cast<char*>( buffer ), n, &bytes_read ) )
      return 0;
    else if ( bytes_read < 0 )
      return 0;
    return bytes_read;
  }

  bool write( const void* buffer, size_t n ) override
  {
    unsigned int bytes_sent;

    return _sck.send_nowait( buffer, n, &bytes_sent );
  }

  bool isOpen() override { return _sck.connected(); }
  void close() override { _sck.close(); }

private:
  Clib::Socket _sck;
};

class DapDebugClientThread
{
public:
  DapDebugClientThread( Clib::Socket&& sock )
      : _rw( std::make_shared<SocketReaderWriter>( std::move( sock ) ) ),
        _session( dap::Session::create() )
  {
  }

  void run();

private:
  std::shared_ptr<SocketReaderWriter> _rw;
  std::unique_ptr<dap::Session> _session;
};

void DapDebugClientThread::run()
{
  POLLOG_INFO << "Debug client thread started.\n";

  // Session event handlers
  _session->registerHandler(
      []( const dap::InitializeRequest& )
      {
        POLLOG_INFO << "Got InitializeRequest.\n";
        return dap::InitializeResponse{};
      } );

  // Attach the SocketReaderWriter to the Session and begin processing events.
  _session->bind( _rw, []() { POLLOG_INFO << "Debug session endpoint closed.\n"; } );

  while ( !Clib::exit_signalled && _rw->isOpen() )
  {
    pol_sleep_ms( 1000 );
  }

  _session.reset();

  // Close the socket endpoint if necessary.
  if ( _rw->isOpen() )
  {
    _rw->close();
  }

  POLLOG_INFO << "Debug client thread closing.\n";
}

void DapDebugServer::dap_debug_listen_thread( void )
{
  if ( Plib::systemstate.config.dap_debug_port )
  {
    Clib::SocketListener SL( Plib::systemstate.config.dap_debug_port );
    while ( !Clib::exit_signalled )
    {
      Clib::Socket sock;
      if ( SL.GetConnection( &sock, 1 ) && sock.connected() )
      {
        auto client = new DapDebugClientThread( std::move( sock ) );
        Core::networkManager.auxthreadpool->push(
            [client]()
            {
              std::unique_ptr<DapDebugClientThread> _clientptr( client );
              _clientptr->run();
            } );
      }
    }
  }
}
}  // namespace Core
}  // namespace Pol