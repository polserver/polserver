/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of boat related cpp/h
 */


#include <chrono>
#include <string.h>
#include <string>
#include <thread>

#include "../clib/esignal.h"
#include "../clib/logfacility.h"
#include "../clib/socketsvc.h"
#include "../clib/strutil.h"
#include "../clib/threadhelp.h"
#include "../clib/wnsckt.h"
#include "../plib/systemstate.h"
#include "core.h"
#include "globals/network.h"
#include "network/client.h"
#include "network/cliface.h"
#include "polsem.h"
#include "uoclient.h"
#include <format/format.h>

namespace Pol
{
namespace Core
{
UoClientThread::UoClientThread( UoClientListener* def, Clib::SocketListener& SL )
    : Clib::SocketClientThread( SL ), _def( def ), client( nullptr )
{
}
UoClientThread::UoClientThread( UoClientThread& copy )
    : Clib::SocketClientThread( copy._sck ), _def( copy._def ), client( copy.client )
{
}

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
        new Network::Client( *Core::networkManager.uo_client_interface.get(), _def->encryption );
    client->csocket = _sck.release_handle();  // client cleans up its socket.
    if ( _def->sticky )
      client->listen_port = _def->port;
    if ( _def->aosresist )
      client->aosresist = true;  // UOCLient.cfg Entry
    // Added null setting for pre-char selection checks using nullptr validation
    client->acct = nullptr;
    memcpy( &client->ipaddr, &client_addr, sizeof client->ipaddr );

    networkManager.clients.push_back( client );
    CoreSetSysTrayToolTip( Clib::tostring( networkManager.clients.size() ) + " clients connected",
                           ToolTipPrioritySystem );
    fmt::Writer tmp;
    tmp.Format( "Client#{} connected from {} ({}/{} connections)" )
        << client->instance_ << Network::AddressToString( &client_addr )
        << networkManager.clients.size() << networkManager.getNumberOfLoginClients();
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
  while ( !Clib::exit_signalled )
  {
    unsigned int timeout = 2;
    unsigned int utimeout = 0;
    if ( !ls->login_clients.empty() )
    {
      timeout = 0;
      utimeout = 200000;
    }
    if ( SL.GetConnection( timeout, utimeout ) )
    {
      // create an appropriate Client object
      if ( Plib::systemstate.config.use_single_thread_login )
      {
        std::unique_ptr<UoClientThread> thread( new UoClientThread( ls, SL ) );
        thread->create();
        client_io_thread( thread->client, true );
        ls->login_clients.push_back( std::move( thread ) );
      }
      else
      {
        Clib::SocketClientThread* thread = new UoClientThread( ls, SL );
        thread->start();
      }
    }

    auto itr = ls->login_clients.begin();
    while ( itr != ls->login_clients.end() )
    {
      if ( ( *itr )->client != nullptr && ( *itr )->client->isReallyConnected() )
      {
        if ( !client_io_thread( ( *itr )->client, true ) )
        {
          itr = ls->login_clients.erase( itr );
          continue;
        }

        if ( ( *itr )->client->isConnected() && ( *itr )->client->chr )
        {
          Clib::SocketClientThread::start_thread( itr->release() );
          itr = ls->login_clients.erase( itr );
        }
        else
        {
          ++itr;
        }
      }
      else
      {
        itr = ls->login_clients.erase( itr );
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
}  // namespace Core
}  // namespace Pol
