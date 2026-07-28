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

#include "clib/esignal.h"
#include "clib/logfacility.h"
#include "clib/network/multipoller.h"
#include "clib/network/socketsvc.h"
#include "clib/network/wnsckt.h"
#include "clib/strutil.h"
#include "clib/threadhelp.h"
#include "plib/systemstate.h"
#include "pol/core.h"
#include "pol/globals/network.h"
#include "pol/network/client.h"
#include "pol/network/clientthread.h"
#include "pol/network/clienttransmit.h"
#include "pol/network/cliface.h"
#include "pol/polclock.h"
#include "pol/polsem.h"
#include "pol/uoclient.h"


namespace Pol::Core
{
using namespace std::chrono_literals;

UoClientThread::UoClientThread( UoClientListener* def, Clib::Socket&& newsck )
    : Clib::SocketClientThread( std::move( newsck ) ),
      _def( def ),
      client( nullptr ),
      login_time( 0 )
{
}

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
  client = new Network::Client( *Core::networkManager.uo_client_interface.get(), _def->encryption,
                                client_addr, _def->allowed_proxies );

  // TODO: move this into an initialization of ThreadedClient.
  client->csocket = _sck.release_handle();  // client cleans up its socket.

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

void UoClientListener::accept_connection( Clib::Socket&& newsck )
{
  // create an appropriate Client object
  if ( Plib::systemstate.config.use_single_thread_login )
  {
    std::unique_ptr<UoClientThread> thread( new UoClientThread( this, std::move( newsck ) ) );
    if ( thread->create() )
    {
      // Nothing has been waited on yet - a connection this new cannot have
      // sent anything, and the next pass polls it along with the others.
      if ( client_io_login_step( thread->client, SocketReadiness{ .timed_out = true } ) )
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

void UoClientListener::run()
{
  INFO_PRINTLN( "Listening for UO clients on port {} (encryption: {},{:#x},{:#x})", port,
                encryption.eType, encryption.uiKey1, encryption.uiKey2 );

  Clib::SocketListener SL(
      port, Clib::Socket::option( Clib::Socket::nonblocking | Clib::Socket::reuseaddr ) );

  // The listen socket and every client still on the login server, waited on
  // together. Waiting on them one at a time meant a client's packet sat
  // untouched until every client before it had been polled in turn, and each
  // of those waits costs a whole timer tick on Windows however short a timeout
  // it is given. Rebuilt every pass, since which clients are here changes.
  Clib::MultiPoller poller;
  std::vector<size_t> client_slots;
  // for a login client that has no socket to wait on; MultiPoller reports
  // nothing ready for an index it does not know
  constexpr size_t no_slot = static_cast<size_t>( -1 );

  while ( !Clib::exit_signalled )
  {
    poller.clear();
    client_slots.clear();
    poller.add( SL.listen_socket(), false );

    for ( const auto& login_client : login_clients )
    {
      auto* client = login_client->client;
      if ( client == nullptr || !client->isReallyConnected() )
      {
        // no socket to wait on; the walk below drops it
        client_slots.push_back( no_slot );
        continue;
      }
      client_slots.push_back(
          poller.add( client->session()->csocket, client->session()->have_queued_data() ) );
    }

    // Only bounds how often the timeout bookkeeping below runs: the wait ends
    // as soon as any socket has something, so this is not a latency floor.
    int res = poller.wait( 1000ms );
    if ( res < 0 && Clib::socket_errno() != Clib::sockerr::intr )
    {
      // Nothing was waited on, so go round again rather than spinning on a
      // listener that has stopped working.
      POLLOG_ERRORLN( "Listener on port {}: poll failed, sckerr={}", port, Clib::socket_errno() );
      pol_sleep_ms( 1000 );
      continue;
    }

    if ( poller.result( 0 ).incoming )
    {
      // Take every connection that is already queued, not just one per pass.
      // Accepting a single one per pass costs a full pass per connection, so a
      // burst of N clients used to be admitted in O(N^2) time - measured at
      // 28.7s for 75 clients on loopback. The cap bounds how long a connection
      // flood can delay the login clients handled below.
      constexpr int max_accepts_per_pass = 64;
      for ( int accepted = 0; accepted < max_accepts_per_pass; ++accepted )
      {
        Clib::Socket newsck;
        if ( !SL.Accept( &newsck ) || !newsck.connected() )
          break;

        accept_connection( std::move( newsck ) );
      }
    }

    size_t slot = 0;
    auto itr = login_clients.begin();
    while ( itr != login_clients.end() )
    {
      auto client = ( *itr )->client;
      // Clients accepted just above were appended after the slots were built,
      // so they have none; they are polled from the next pass on.
      const auto result =
          slot < client_slots.size() ? poller.result( client_slots[slot] ) : Clib::PollResult{};
      ++slot;
      if ( client != nullptr && client->isReallyConnected() )
      {
        // Every login client gets a pass, whether or not its socket had
        // anything, so the bookkeeping below keeps running at the rate the
        // wait's timeout sets.
        SocketReadiness ready{ result.incoming, result.writable, result.error,
                               !result.incoming && !result.writable && !result.error };
        if ( !client_io_login_step( client, ready ) )
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

void start_uo_client_listeners()
{
  for ( auto& uoclient_listener : networkManager.uoclient_listeners )
  {
    UoClientListener* ls = &uoclient_listener;
    std::string threadname = "UO Client Listener Port " + Clib::tostring( ls->port );
    threadhelp::start_thread( uo_client_listener_thread, threadname.c_str(), ls );
  }
}
}  // namespace Pol::Core
