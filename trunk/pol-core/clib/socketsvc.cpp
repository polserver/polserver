/*
History
=======

Notes
=======

*/

#include "stl_inc.h"

#ifdef _WIN32
#include <windows.h> // for HANDLE
#include <process.h>
#endif

#include "passert.h"
#include "strutil.h"
#include "threadhelp.h"

#include "socketsvc.h"

SocketListener::SocketListener( unsigned short port ) :
    _listen_sck()
{
    if (!_listen_sck.listen(port))
        throw runtime_error( "Unable to open listen port " + decint(port));
}

SocketListener::SocketListener( unsigned short port, Socket::option opt ) :
    _listen_sck()
{
    _listen_sck.set_options( opt );

    if (!_listen_sck.listen(port))
        throw runtime_error( "Unable to open listen port " + decint(port) );
}

bool SocketListener::GetConnection( unsigned int timeout_sec )
{
    return _listen_sck.select( timeout_sec, 0 );
}

void SocketListener::accept( Socket& newsck )
{
    _listen_sck.accept( newsck ); // FIXME return ignored
}


SocketClientThread::SocketClientThread( SocketListener& SL ) :
    _sck()
{
    SL.accept( _sck );
}
SocketClientThread::SocketClientThread( Socket& S ) :
    _sck(S)
{
}

static void _thread_stub2( void *arg )
{
    auto_ptr<SocketClientThread> sct( static_cast<SocketClientThread*>(arg) );
    sct->run();
}

void SocketClientThread::start()
{
    threadhelp::start_thread( _thread_stub2, "SocketClientThread", this );
}
