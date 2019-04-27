#include "network.h"

#include <curl/curl.h>
#include <string.h>

#include "../../clib/logfacility.h"
#include "../../clib/stlutil.h"
#include "../../clib/threadhelp.h"
#include "../../plib/systemstate.h"
#include "../accounts/account.h"
#include "../mobile/charactr.h"
#include "../network/auxclient.h"
#include "../network/clienttransmit.h"
#include "../network/cliface.h"
#include "../network/msgfiltr.h"
#include "../network/msghandl.h"
#include "../network/packethooks.h"
#include "../network/packetinterface.h"
#include "../network/sockio.h"
#include "../servdesc.h"
#include "../sqlscrobj.h"
#include "../uoclient.h"

namespace Pol
{
namespace Core
{
NetworkManager networkManager;

NetworkManager::NetworkManager()
    : clients(),
      servers(),
      polstats(),
#ifdef HAVE_MYSQL
      sql_service( new SQLService ),
#endif
      uo_client_interface( new Network::UOClientInterface() ),
      auxservices(),
      uoclient_general(),
      uoclient_protocol(),
      uoclient_listeners(),
      iostats(),
      queuedmode_iostats(),
      login_filter( nullptr ),
      game_filter( nullptr ),
      disconnected_filter( nullptr ),
      packet_hook_data(),
      packet_hook_data_v2(),
      handler(),
      handler_v2(),
      ext_handler_table(),
      packetsSingleton( new Network::PacketsSingleton() ),
      clientTransmit( new Network::ClientTransmit() ),
      auxthreadpool( new threadhelp::DynTaskThreadPool( "AuxPool" ) ),  // TODO: seems to work
                                                                        // activate by default?
                                                                        // maybe add a cfg entry for
                                                                        // max number of threads
      banned_ips()
{
  memset( ipaddr_str, 0, sizeof ipaddr_str );
  memset( lanaddr_str, 0, sizeof lanaddr_str );
  memset( hostname, 0, sizeof hostname );
  MessageTypeFilter::createMessageFilter();
  Network::PacketHookData::initializeGameData( &packet_hook_data );
  Network::PacketHookData::initializeGameData( &packet_hook_data_v2 );

  Network::PacketRegistry::initialize_msg_handlers();

  curl_global_init( CURL_GLOBAL_DEFAULT );
}

NetworkManager::~NetworkManager() {}
void NetworkManager::kill_disconnected_clients()
{
  Clients::iterator itr = clients.begin();
  while ( itr != clients.end() )
  {
    Network::Client* client = *itr;
    if ( !client->isReallyConnected() )
    {
      fmt::Writer tmp;
      tmp.Format( "Disconnecting Client#{} ({}/{})" )
          << client->instance_ << ( client->acct ? client->acct->name() : "[no account]" )
          << ( client->chr ? client->chr->name() : "[no character]" );
      ERROR_PRINT << tmp.str() << "\n";
      if ( Plib::systemstate.config.loglevel >= 4 )
        POLLOG << tmp.str() << "\n";

      Network::Client::Delete( client );
      client = nullptr;
      itr = clients.erase( itr );
    }
    else
    {
      ++itr;
    }
  }
}

void NetworkManager::deinialize()
{
  for ( auto& client : clients )
  {
    client->forceDisconnect();
  }
  kill_disconnected_clients();
  for ( auto& client : clients )
  {
    Network::Client::Delete( client );
  }
  clients.clear();

  Clib::delete_all( servers );

  // unload_aux_service
  Clib::delete_all( auxservices );
  auxthreadpool.reset();
  banned_ips.clear();

  Network::deinit_sockets_library();
  Network::clean_packethooks();
  curl_global_cleanup();
  uoclient_general.deinitialize();
}

size_t NetworkManager::getNumberOfLoginClients() const
{
  size_t no = 0;
  for ( const auto& ls : uoclient_listeners )
    no += ls.login_clients.size();
  return no;
}

NetworkManager::Memory NetworkManager::estimateSize() const
{
  Memory usage;
  memset( &usage, 0, sizeof( usage ) );
  usage.misc = sizeof( NetworkManager );

  usage.client_size =
      3 * sizeof( Network::Client** ) + clients.capacity() * sizeof( Network::Client* );
  usage.client_count = clients.size();
  for ( const auto& client : clients )
  {
    if ( client != nullptr )
      usage.client_size += client->estimatedSize();
  }

  usage.misc +=
      3 * sizeof( ServerDescription** ) + servers.capacity() * sizeof( ServerDescription* );
  for ( const auto& server : servers )
    if ( server != nullptr )
      usage.misc += server->estimateSize();

#ifdef HAVE_MYSQL
  usage.misc += sizeof( SQLService ); /* sql_service */
#endif
  usage.misc += sizeof( Network::UOClientInterface ); /*uo_client_interface*/
  usage.misc +=
      3 * sizeof( Network::AuxService** ) + auxservices.capacity() * sizeof( Network::AuxService* );
  for ( const auto& aux : auxservices )
    if ( aux != nullptr )
      usage.misc += aux->estimateSize();

  usage.misc += uoclient_general.estimateSize();
  usage.misc += uoclient_protocol.estimateSize();
  usage.misc += 3 * sizeof( UoClientListener* );
  for ( const auto& listener : uoclient_listeners )
    usage.misc += listener.estimateSize();

  usage.misc +=
      3 * ( sizeof( MessageTypeFilter ) ); /*login_filter, game_filter, disconnected_filter*/

  usage.misc += 3 * sizeof( std::unique_ptr<Network::PacketHookData>* ) +
                packet_hook_data.capacity() * sizeof( std::unique_ptr<Network::PacketHookData> );
  usage.misc += 3 * sizeof( std::unique_ptr<Network::PacketHookData>* ) +
                packet_hook_data_v2.capacity() * sizeof( std::unique_ptr<Network::PacketHookData> );
  for ( const auto& hook : packet_hook_data )
  {
    if ( hook != nullptr )
      usage.misc += hook->estimateSize();
  }
  for ( const auto& hook : packet_hook_data_v2 )
  {
    if ( hook != nullptr )
      usage.misc += hook->estimateSize();
  }

  usage.misc += packetsSingleton->estimateSize();
  usage.misc += sizeof( Network::ClientTransmit );
  usage.misc += sizeof( threadhelp::DynTaskThreadPool );

  usage.misc += 3 * sizeof( Network::IPRule* ) + banned_ips.capacity() * sizeof( Network::IPRule );

  return usage;
}
}  // namespace Core
}  // namespace Pol
