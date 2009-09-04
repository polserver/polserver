/*
History
=======
2009/09/03 MuadDib:   Relocation of boat related cpp/h


Notes
=======

*/

#include "clib/stl_inc.h"

#include "clib/esignal.h"
#include "clib/logfile.h"
#include "clib/socketsvc.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"
#include "clib/threadhelp.h"

#include "client.h"
#include "cliface.h"
#include "core.h"
#include "uoclient.h"
#include "polsem.h"
#include "sockio.h"
#include "uvars.h"

class UoClientThread : public SocketClientThread
{
public:
    UoClientThread( UoClientListener* def, SocketListener& SL ) : 
        SocketClientThread(SL),_def(*def) {}
    virtual void run();

private:
    UoClientListener _def;
};

void client_io_thread( Client* client );

void UoClientThread::run()
{
    Client* client = NULL;

    {
        struct sockaddr client_addr = _sck.peer_address();
        struct sockaddr host_addr;
        socklen_t host_addrlen = sizeof host_addr;

        PolLock lck;
        client = new Client( uo_client_interface, _def.encryption );
        client->csocket = _sck.release_handle(); // client cleans up its socket.
        client->listen_port = static_cast<unsigned short>(_def.port);
		if ( _def.aosresist )
			client->aosresist = true; // UOCLient.cfg Entry
		// Added null setting for pre-char selection checks using NULL validation
		client->acct = NULL;
        memcpy( &client->ipaddr, &client_addr, sizeof client->ipaddr );

        clients.push_back( client );
        CoreSetSysTrayToolTip( tostring(clients.size()) + " clients connected", ToolTipPrioritySystem );
        cout << "Client connected from " << AddressToString( &client_addr )
                << " (" << clients.size() << " connections)"
                << endl;
        string ifdesc = "";
        if (getsockname( client->csocket, &host_addr, &host_addrlen ) == 0)
        {
            // cout << "host address: " << AddressToString( &host_addr ) << endl;
            ifdesc = " on interface ";
            ifdesc += AddressToString( &host_addr );
        }
        Log( "Client#%lu connected from %s (%d connections)%s\n", 
                client->instance_,
                AddressToString( &client_addr ),
                clients.size(),
                ifdesc.c_str() );
    }

    client_io_thread( client );
}

void uo_client_listener_thread( void* arg )
{
    UoClientListener* ls = reinterpret_cast<UoClientListener*>(arg);

    atomic_cout( "Listening for UO clients on port " + decint(ls->port) + " (encryption: " + ls->encryption + ")");

    SocketListener SL( ls->port, Socket::option(Socket::nonblocking|Socket::reuseaddr) );
    while (!exit_signalled)
    {
        if (SL.GetConnection( 5 ))
        {
            // create an appropriate Client object

            SocketClientThread* thread = new UoClientThread( ls, SL );
            thread->start();
        }
    }
}

void start_uo_client_listeners( void )
{
    for( unsigned i = 0; i < uoclient_listeners.size(); ++i )
    {
        UoClientListener* ls = &uoclient_listeners[ i ];
        string threadname = "UO Client Listener Port " + tostring( ls->port );
        threadhelp::start_thread( uo_client_listener_thread, threadname.c_str(), ls );
    }
}
