/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of boat related cpp/h
 */


#include <string.h>
#include <string>
#include <sys/socket.h>

#include "../../lib/format/format.h"
#include "../clib/compilerspecifics.h"
#include "../clib/esignal.h"
#include "../clib/logfacility.h"
#include "../clib/socketsvc.h"
#include "../clib/stlutil.h"
#include "../clib/threadhelp.h"
#include "../clib/wnsckt.h"
#include "../plib/systemstate.h"
#include "core.h"
#include "globals/network.h"
#include "network/client.h"
#include "network/cliface.h"
#include "polsem.h"
#include "uoclient.h"

namespace Pol
{
namespace Core
{
class UoClientThread : public Clib::SocketClientThread
{
public:
  UoClientThread( UoClientListener* def, Clib::SocketListener& SL )
      : Clib::SocketClientThread( SL ), _def( *def ), client( NULL )
  {
  }
  UoClientThread( UoClientThread& copy )
      : Clib::SocketClientThread( copy._sck ), _def( copy._def ), client( copy.client ){};
  virtual void run() POL_OVERRIDE;
  void create();
  virtual ~UoClientThread(){};

private:
  UoClientListener _def;

public:
  Network::Client* client;
};

bool client_io_thread( Network::Client* client, bool login = false );

void UoClientThread::run()
{
  if ( !Plib::systemstate.config.use_single_thread_login )
  {
    create();
  }
  client->thread_pid = threadhelp::thread_pid();
  client_io_thread( client );
}

void UoClientThread::create()
{
  {
    if ( Plib::systemstate.config.disable_nagle )
    {
      _sck.disable_nagle();
    }
    struct sockaddr client_addr = _sck.peer_address();
    struct sockaddr host_addr;
    socklen_t host_addrlen = sizeof host_addr;

    PolLock lck;
    client =
        new Network::Client( *Core::networkManager.uo_client_interface.get(), _def.encryption );
    client->csocket = _sck.release_handle();  // client cleans up its socket.
    if ( _def.sticky )
      client->listen_port = _def.port;
    if ( _def.aosresist )
      client->aosresist = true;  // UOCLient.cfg Entry
    // Added null setting for pre-char selection checks using NULL validation
    client->acct = NULL;
    memcpy( &client->ipaddr, &client_addr, sizeof client->ipaddr );

    networkManager.clients.push_back( client );
    CoreSetSysTrayToolTip( Clib::tostring( networkManager.clients.size() ) + " clients connected",
                           ToolTipPrioritySystem );
    fmt::Writer tmp;
    tmp.Format( "Client#{} connected from {} ({} connections)" )
        << client->instance_ << Network::AddressToString( &client_addr )
        << networkManager.clients.size();
    if ( getsockname( client->csocket, &host_addr, &host_addrlen ) == 0 )
    {
      tmp << " on interface " << Network::AddressToString( &host_addr );
    }
    POLLOG << tmp.str() << "\n";
  }
}


void uo_client_listener_thread( void* arg )
{
  UoClientListener* ls = static_cast<UoClientListener*>( arg );
  INFO_PRINT << "Listening for UO clients on port " << ls->port
             << " (encryption: " << ls->encryption.eType << ",0x"
             << fmt::hexu( ls->encryption.uiKey1 ) << ",0x" << fmt::hexu( ls->encryption.uiKey2 )
             << ")\n";

  Clib::SocketListener SL(
      ls->port, Clib::Socket::option( Clib::Socket::nonblocking | Clib::Socket::reuseaddr ) );
  std::list<std::unique_ptr<UoClientThread>> login_clients;
  while ( !Clib::exit_signalled )
  {
    unsigned int timeout = 2;
    if ( !login_clients.empty() )
      timeout = 0;
    if ( SL.GetConnection( timeout ) )
    {
      // create an appropriate Client object
      if ( Plib::systemstate.config.use_single_thread_login )
      {
        std::unique_ptr<UoClientThread> thread( new UoClientThread( ls, SL ) );
        thread->create();
        client_io_thread( thread->client, true );
        login_clients.push_back( std::move( thread ) );
      }
      else
      {
        Clib::SocketClientThread* thread = new UoClientThread( ls, SL );
        thread->start();
      }
    }

    auto itr = login_clients.begin();
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
          Clib::SocketClientThread::start_thread( itr->release() );
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
  for ( unsigned i = 0; i < networkManager.uoclient_listeners.size(); ++i )
  {
    UoClientListener* ls = &networkManager.uoclient_listeners[i];
    std::string threadname = "UO Client Listener Port " + Clib::tostring( ls->port );
    threadhelp::start_thread( uo_client_listener_thread, threadname.c_str(), ls );
  }
}
}
}
