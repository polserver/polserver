/** @file
 *
 * @par History
 */


#include <stdlib.h>

#include "logfacility.h"
#include "socketsvc.h"
#include "strutil.h"
#include "threadhelp.h"

namespace Pol
{
namespace Clib
{
SocketListener::SocketListener( unsigned short port ) : _listen_sck()
{
  if ( !_listen_sck.listen( port ) )
  {
    POLLOG_ERRORLN( "Unable to open listen port {}", port );
    abort();
  }
}

SocketListener::SocketListener( unsigned short port, Socket::option opt ) : _listen_sck()
{
  _listen_sck.set_options( opt );

  if ( !_listen_sck.listen( port ) )
  {
    POLLOG_ERRORLN( "Unable to open listen port {}", port );
    abort();
  }
}

bool SocketListener::GetConnection( Socket* newsck, unsigned int timeout_sec,
                                    unsigned int timeout_msec )
{
  if ( _listen_sck.has_incoming_data( timeout_sec * 1000 + timeout_msec ) )
    return _listen_sck.accept( newsck );
  return false;
}

SocketClientThread::SocketClientThread( Socket&& S ) : _sck( std::move( S ) ) {}

static void _thread_stub2( void* arg )
{
  std::unique_ptr<SocketClientThread> sct( static_cast<SocketClientThread*>( arg ) );
  sct->run();
}

void SocketClientThread::start()
{
  threadhelp::start_thread( _thread_stub2, "SocketClientThread", this );
}

void SocketClientThread::start_thread( SocketClientThread* instance )
{
  threadhelp::start_thread( _thread_stub2, "SocketClientThread", instance );
}
}  // namespace Clib
}  // namespace Pol
