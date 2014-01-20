/*
History
=======
2009/09/03 MuadDib:   Relocation of boat related cpp/h


Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/esignal.h"
#include "../clib/logfacility.h"
#include "../clib/socketsvc.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/threadhelp.h"

#include "network/client.h"
#include "network/cliface.h"
#include "core.h"
#include "uoclient.h"
#include "polsem.h"
#include "sockio.h"
#include "uvars.h"
#include "polcfg.h"
namespace Pol {
  namespace Core {
	class UoClientThread : public Clib::SocketClientThread
	{
	public:
      UoClientThread( UoClientListener* def, Clib::SocketListener& SL ) :
        Clib::SocketClientThread( SL ), _def( *def ), client( NULL )
	  {}
      UoClientThread( UoClientThread& copy ) : Clib::SocketClientThread( copy._sck ), _def( copy._def ), client( copy.client ) {}
	  virtual void run();
	  void create();
	  ~UoClientThread() {}

	private:
	  UoClientListener _def;
	public:
	  Network::Client* client;
	};

    bool client_io_thread( Network::Client* client, bool login = false );

	void UoClientThread::run()
	{
		if ( config.disable_nagle) {
			_sck.disable_nagle();
		}
	  if ( !config.use_single_thread_login )
	  {
		create();
	  }
	  client->thread_pid = threadhelp::thread_pid();
	  client_io_thread( client );
	}

	void UoClientThread::create()
	{
	  {
		struct sockaddr client_addr = _sck.peer_address();
		struct sockaddr host_addr;
		socklen_t host_addrlen = sizeof host_addr;

		PolLock lck;
        client = new Network::Client( Network::uo_client_interface, _def.encryption );
		client->csocket = _sck.release_handle(); // client cleans up its socket.
		if ( _def.sticky )
		  client->listen_port = _def.port;
		if ( _def.aosresist )
		  client->aosresist = true; // UOCLient.cfg Entry
		// Added null setting for pre-char selection checks using NULL validation
		client->acct = NULL;
		memcpy( &client->ipaddr, &client_addr, sizeof client->ipaddr );

		clients.push_back( client );
		CoreSetSysTrayToolTip( Clib::tostring( clients.size() ) + " clients connected", ToolTipPrioritySystem );
        fmt::Writer tmp;
        tmp.Format( "Client#{} connected from {} ({} connections)" )
          << client->instance_
          << Network::AddressToString( &client_addr )
          << clients.size();
		if ( getsockname( client->csocket, &host_addr, &host_addrlen ) == 0 )
		{
          tmp << " on interface " << Network::AddressToString( &host_addr );
		}
        POLLOG << tmp.c_str() << "\n";
	  }
	}


	void uo_client_listener_thread( void* arg )
	{
	  UoClientListener* ls = static_cast<UoClientListener*>( arg );
      INFO_PRINT << "Listening for UO clients on port " << ls->port
        << " (encryption: " << ls->encryption.eType << ",0x" << fmt::hexu( ls->encryption.uiKey1 ) << ",0x" << fmt::hexu( ls->encryption.uiKey2 ) << ")\n";

      Clib::SocketListener SL( ls->port, Clib::Socket::option( Clib::Socket::nonblocking | Clib::Socket::reuseaddr ) );
	  list<UoClientThread *> login_clients;
      while ( !Clib::exit_signalled )
	  {
		unsigned int timeout = 2;
		if ( !login_clients.empty() )
		  timeout = 1;
		if ( SL.GetConnection( timeout ) )
		{
		  // create an appropriate Client object
		  if ( config.use_single_thread_login )
		  {
			UoClientThread* thread = new UoClientThread( ls, SL );
			login_clients.push_back( thread );
			thread->create();
			client_io_thread( thread->client, true );
		  }
		  else
		  {
            Clib::SocketClientThread* thread = new UoClientThread( ls, SL );
			thread->start();
		  }
		}
		list<UoClientThread*>::iterator itr = login_clients.begin();
		while ( itr != login_clients.end() )
		{
		  if ( ( *itr )->client != NULL && ( *itr )->client->isReallyConnected() )
		  {
			if ( !client_io_thread( ( *itr )->client, true ) )
			{
			  itr = login_clients.erase( itr );
			  continue;
			}

			if ( ( *itr )->client->isConnected() && ( *itr )->client->chr )
			{
              Clib::SocketClientThread::start_thread( new UoClientThread( *( *itr ) ) );
			  itr = login_clients.erase( itr );
			}
			else
			{
			  ++itr;
			}
		  }
		  else
		  {
			itr = login_clients.erase( itr );
		  }
		}
	  }
	}

	void start_uo_client_listeners( void )
	{
	  for ( unsigned i = 0; i < uoclient_listeners.size(); ++i )
	  {
		UoClientListener* ls = &uoclient_listeners[i];
        string threadname = "UO Client Listener Port " + Clib::tostring( ls->port );
		threadhelp::start_thread( uo_client_listener_thread, threadname.c_str(), ls );
	  }
	}
  }
}