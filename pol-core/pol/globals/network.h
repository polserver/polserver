#ifndef GLOBALS_NETWORK_H
#define GLOBALS_NETWORK_H

#include "pol_global_config.h"

#include <array>
#include <memory>
#include <vector>

#include "../network/bannedips.h"
#include "../network/iostats.h"
#include "../network/msghandl.h"
#include "../network/sockio.h"
#include "../polstats.h"
#include "../uoclient.h"

namespace Pol
{
namespace Network
{
namespace DAP
{
class DebugServer;
}
class AuxService;
class Client;
class ClientTransmit;
class PacketHookData;
class PacketsSingleton;
class UOClientInterface;
}  // namespace Network
namespace threadhelp
{
class DynTaskThreadPool;
}
namespace Core
{
class MessageTypeFilter;
class SQLService;
class ServerDescription;

typedef std::vector<Network::Client*> Clients;
typedef std::vector<ServerDescription*> Servers;

class NetworkManager
{
public:
  NetworkManager();
  ~NetworkManager();
  NetworkManager( const NetworkManager& ) = delete;
  NetworkManager& operator=( const NetworkManager& ) = delete;

  void initialize();
  void deinialize();
  void kill_disconnected_clients();

  size_t getNumberOfLoginClients() const;
  struct Memory;

  Memory estimateSize() const;

  Clients clients;
  Servers servers;

  PolStats polstats;

#ifdef HAVE_MYSQL
  std::unique_ptr<SQLService> sql_service;
#endif
  std::unique_ptr<Network::UOClientInterface> uo_client_interface;

  std::vector<Network::AuxService*> auxservices;

  UoClientGeneral uoclient_general;
  UoClientProtocol uoclient_protocol;
  std::vector<UoClientListener> uoclient_listeners;

  Network::IOStats iostats;
  Network::IOStats queuedmode_iostats;
  std::unique_ptr<MessageTypeFilter> login_filter;
  std::unique_ptr<MessageTypeFilter> game_filter;
  std::unique_ptr<MessageTypeFilter> disconnected_filter;

  // stores information about each packet and its script & default handler
  std::vector<std::unique_ptr<Network::PacketHookData>> packet_hook_data;
  std::vector<std::unique_ptr<Network::PacketHookData>> packet_hook_data_v2;
  // handler[] is used for storing the core MSG_HANDLER calls.
  std::array<Network::MSG_HANDLER, 256> handler;
  /*
  handler_v2[] is used for storing the core MSG_HANDLER calls for packets that
  were changed in client 6.0.1.7 (or any newer version where a second handler is
  required due to changed incoming packet structure).
  */
  std::array<Network::MSG_HANDLER, 256> handler_v2;

  std::array<Network::ExtMsgHandler, 256> ext_handler_table;

  char ipaddr_str[64];
  char lanaddr_str[64];
  char hostname[64];

  std::unique_ptr<Network::PacketsSingleton> packetsSingleton;

  std::unique_ptr<Network::ClientTransmit> clientTransmit;

  std::unique_ptr<threadhelp::DynTaskThreadPool> auxthreadpool;

  std::vector<Network::IPRule> banned_ips;

  struct Memory
  {
    size_t client_size;
    size_t client_count;
    size_t misc;
  };

private:
  std::unique_ptr<Network::DAP::DebugServer> _dap_debug_server;
};

extern NetworkManager networkManager;
}  // namespace Core
}  // namespace Pol
#endif
