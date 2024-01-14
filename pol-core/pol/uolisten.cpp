/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of boat related cpp/h
 */


#include <chrono>
#include <iterator>
#include <string.h>
#include <string>
#include <thread>

#include "../clib/esignal.h"
#include "../clib/logfacility.h"
#include "../clib/network/socketsvc.h"
#include "../clib/network/wnsckt.h"
#include "../clib/strutil.h"
#include "../clib/threadhelp.h"
#include "../plib/systemstate.h"
#include "core.h"
#include "globals/network.h"
#include "network/client.h"
#include "network/clienttransmit.h"
#include "network/cliface.h"
#include "polsem.h"
#include "uoclient.h"
#include <format/format.h>

namespace Pol
{
namespace Core
{
UoClientThread::UoClientThread( UoClientListener* def, Clib::Socket&& newsck )
    : Clib::SocketClientThread( std::move( newsck ) ),
      _def( def ),
      client( nullptr ),
      login_time( 0 )
{
}

bool client_io_thread( Network::Client* client, bool login = false );

void UoClientThread::run()
{
  if ( !Plib::systemstate.config.use_single_thread_login )
  {
    if ( !create() )
      return;
  }
  client->session()->thread_pid = threadhelp::thread_pid();
  client_io_thread( client );
}

bool UoClientThread::create()
{
  if ( !_sck.connected() )  // should not happend, just here to be sure
  {
    POLLOGLN( "Login failed, socket is invalid" );
    return false;
  }
  login_time = poltime();
  if ( Plib::systemstate.config.disable_nagle )
  {
    _sck.disable_nagle();
  }
  struct sockaddr client_addr = _sck.peer_address();
  struct sockaddr host_addr;
  socklen_t host_addrlen = sizeof host_addr;

  PolLock lck;
  client = new Network::Client( *Core::networkManager.uo_client_interface.get(), _def->encryption );

  // TODO: move this into an initialization of ThreadedClient.
  client->csocket = _sck.release_handle();  // client cleans up its socket.
  memcpy( &client->ipaddr, &client_addr, sizeof client->ipaddr );

  if ( _def->sticky )
    client->listen_port = _def->port;
  if ( _def->aosresist )
    client->aosresist = true;  // UOCLient.cfg Entry
  // Added null setting for pre-char selection checks using nullptr validation
  client->acct = nullptr;

  networkManager.clients.push_back( client );
  CoreSetSysTrayToolTip( Clib::tostring( networkManager.clients.size() ) + " clients connected",
                         ToolTipPrioritySystem );
  std::string tmp =
      fmt::format( "Client#{} connected from {} ({}/{} connections)", client->instance_,
                   Network::AddressToString( &client_addr ), networkManager.clients.size(),
                   networkManager.getNumberOfLoginClients() );
  if ( getsockname( client->csocket, &host_addr, &host_addrlen ) == 0 )
  {
    fmt::format_to( std::back_inserter( tmp ), " on interface {}",
                    Network::AddressToString( &host_addr ) );
  }
  POLLOGLN( tmp );
  return true;
}


void uo_client_listener_thread( void* arg )
{
  UoClientListener* ls = static_cast<UoClientListener*>( arg );
  ls->run();
}

void UoClientListener::run()
{
  INFO_PRINTLN( "Listening for UO clients on port {} (encryption: {},{:#x},{:#x})", port,
                encryption.eType, encryption.uiKey1, encryption.uiKey2 );

  Clib::SocketListener SL(
      port, Clib::Socket::option( Clib::Socket::nonblocking | Clib::Socket::reuseaddr ) );
  while ( !Clib::exit_signalled )
  {
    unsigned int timeout = 2;
    unsigned int mstimeout = 0;
    if ( !login_clients.empty() )
    {
      timeout = 0;
      mstimeout = 200;
    }
    Clib::Socket newsck;
    if ( SL.GetConnection( &newsck, timeout, mstimeout ) && newsck.connected() )
    {
      // create an appropriate Client object
      if ( Plib::systemstate.config.use_single_thread_login )
      {
        std::unique_ptr<UoClientThread> thread( new UoClientThread( this, std::move( newsck ) ) );
        if ( thread->create() )
        {
          if ( client_io_thread( thread->client, true ) )
          {
            login_clients.push_back( std::move( thread ) );
            ++login_clients_size;
          }
        }
      }
      else
      {
        Clib::SocketClientThread* thread = new UoClientThread( this, std::move( newsck ) );
        thread->start();
      }
    }

    auto itr = login_clients.begin();
    while ( itr != login_clients.end() )
    {
      auto client = ( *itr )->client;
      if ( client != nullptr && client->isReallyConnected() )
      {
        if ( !client_io_thread( client, true ) )
        {
          itr = login_clients.erase( itr );
          --login_clients_size;
          continue;
        }

        if ( client->isConnected() && client->chr )
        {
          Clib::SocketClientThread::start_thread( itr->release() );
          itr = login_clients.erase( itr );
          --login_clients_size;
        }
        else if ( ( ( *itr )->login_time +
                    Plib::systemstate.config.loginserver_timeout_mins * 60 ) < poltime() )
        {
          POLLOGLN( "Client#{} LoginServer timeout disconnect", client->instance_ );
          PolLock lck;
          client->forceDisconnect();
          client->unregister();
          networkManager.clientTransmit->QueueDelete( client );
          itr = login_clients.erase( itr );
          --login_clients_size;
        }
        else
        {
          ++itr;
        }
      }
      else
      {
        itr = login_clients.erase( itr );
        --login_clients_size;
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
