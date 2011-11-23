/*
History
=======

Notes
=======

*/

#ifndef SOCKETSVC_H
#define SOCKETSVC_H

#include "wnsckt.h"


class SocketListener
{
public:
    explicit SocketListener( unsigned short port );
    SocketListener( unsigned short port, Socket::option opt );
    bool GetConnection( unsigned int timeout_sec );

    void accept( Socket& newsck );

    friend class SocketClientThread;
private:
    Socket _listen_sck;
};

class SocketClientThread
{
public:
    explicit SocketClientThread( SocketListener& SL );
    explicit SocketClientThread( Socket& S );
    virtual ~SocketClientThread() {}
    void start();
    virtual void run() = 0;

protected:
    Socket _sck;
};

#endif
