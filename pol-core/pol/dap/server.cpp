#include "server.h"

#include "../../clib/esignal.h"
#include "../../clib/logfacility.h"
#include "../../clib/network/socketsvc.h"
#include "../../clib/network/wnsckt.h"
#include "../../plib/systemstate.h"
#include "../polclock.h"

#include <dap/io.h>
#include <dap/protocol.h>
#include <dap/session.h>
#include <memory>

namespace Pol
{
namespace Core
{
class Event
{
public:
  // wait() blocks until the event is fired.
  void wait()
  {
    std::unique_lock<std::mutex> lock( mutex );
    cv.wait( lock, [&] { return fired; } );
  }

  // fire() sets signals the event, and unblocks any calls to wait().
  void fire()
  {
    std::unique_lock<std::mutex> lock( mutex );
    fired = true;
    cv.notify_all();
  }

private:
  std::mutex mutex;
  std::condition_variable cv;
  bool fired = false;
};

// Implements the dap::ReaderWriter IO interface using a Clib::Socket
class SocketReaderWriter : public dap::ReaderWriter
{
public:
  SocketReaderWriter( Clib::Socket& sck ) : _sck( sck ) {}
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
  Clib::Socket& _sck;
};

class DapDebugClientThread : public Clib::SocketClientThread
{
public:
  DapDebugClientThread( Clib::Socket&& sock ) : Clib::SocketClientThread( std::move( sock ) ) {}
  void run() override;
};

void DapDebugClientThread::run()
{
  POLLOG_INFO << "Debug client thread started.\n";

  // Create a new dap::Session.
  auto session = dap::Session::create();

  // Session event handlers
  session->registerHandler(
      []( const dap::InitializeRequest& )
      {
        POLLOG_INFO << "Got InitializeRequest.\n";
        return dap::InitializeResponse{};
      } );

  Event sessionClosed;
  // Attach the SocketReaderWrier to the Session and begin processing events.
  session->bind( std::make_shared<SocketReaderWriter>( _sck ),
                 [&]()
                 {
                   POLLOG_INFO << "Debug session endpoint closed.\n";
                   // Signal to client thread that the Session is closed.
                   sessionClosed.fire();
                 } );

  while ( !Clib::exit_signalled && _sck.connected() )
  {
    pol_sleep_ms( 1000 );
  }

  // Close the socket endpoint if necessary.
  if ( _sck.connected() )
  {
    _sck.close();
  }

  // Wait until the Session's closed handler executes.
  sessionClosed.wait();

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
        Clib::SocketClientThread* p = new DapDebugClientThread( std::move( sock ) );
        p->start();
      }
    }
  }
}
}  // namespace Core
}  // namespace Pol