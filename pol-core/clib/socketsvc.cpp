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
    POLLOG_ERROR << "Unable to open listen port " + tostring( port );
    abort();
  }
}

SocketListener::SocketListener( unsigned short port, Socket::option opt ) : _listen_sck()
{
  _listen_sck.set_options( opt );

  if ( !_listen_sck.listen( port ) )
  {
    POLLOG_ERROR << "Unable to open listen port " + tostring( port );
    abort();
  }
}

bool SocketListener::GetConnection( unsigned int timeout_sec, unsigned int timeout_usec )
{
  return _listen_sck.select( timeout_sec, timeout_usec );
}

void SocketListener::accept( Socket& newsck )
{
  _listen_sck.accept( newsck );  // FIXME return ignored
}


SocketClientThread::SocketClientThread( SocketListener& SL ) : _sck()
{
  SL.accept( _sck );
}
SocketClientThread::SocketClientThread( Socket& S ) : _sck( S ) {}

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
