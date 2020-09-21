/** @file
 *
 * @par History
 * - 2005/01/24 Shinigami: added get-/setspyonclient2 to support packet 0xd9 (Spy on Client 2)
 * - 2005/04/03 Shinigami: added UOExpansionFlag for Samurai Empire
 * - 2005/08/29 Shinigami: character.spyonclient2 renamed to character.clientinfo
 *                         get-/setspyonclient2 renamed to get-/setclientinfo
 * - 2006/05/16 Shinigami: added UOExpansionFlag for Mondain's Legacy
 *                         added GENDER/RACE flag (e.g. used inside ClientCreateChar())
 * - 2007/07/09 Shinigami: added isUOKR [bool] - UO:KR client used?
 * - 2009/08/10 MuadDib:   Added CLIENT_VER_50000 for v5.0.0x clients.
 * - 2009/08/19 Turley:    Added u32 UOExpansionFlagClient
 * - 2009/09/06 Turley:    Added u8 ClientType + FlagEnum
 *                         Removed is*
 * - 2009/12/02 Turley:    added SA expansion - Tomi
 * - 2009/12/04 Turley:    Crypto cleanup - Tomi
 * - 2010/01/22 Turley:    Speedhack Prevention System
 * - 2016/11/29 Boberski:  added TOL expansion
 */


#ifndef __CLIENT_H
#define __CLIENT_H

#include <atomic>
#include <cstring>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include "../../clib/network/sockets.h"
#include "../../clib/rawtypes.h"
#include "../../clib/spinlock.h"
#include "../../clib/wallclock.h"
#include "../../clib/weakptr.h"
#include "../../plib/uconst.h"
#include "../crypt/cryptkey.h"
#include "../polclock.h"
#include "pktdef.h"
#include "pktin.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class BStruct;
}  // namespace Bscript
namespace Core
{
class MessageTypeFilter;
struct XmitBuffer;
}  // namespace Core
namespace Accounts
{
class Account;
}
namespace Mobile
{
class Character;
}
namespace Crypt
{
class CCryptBase;
}
namespace Network
{
class ClientGameData;
class ClientInterface;

const u16 T2A = 0x01;
const u16 LBR = 0x02;
const u16 AOS = 0x04;
const u16 SE = 0x08;  // set AOS-Flag in send_feature_enable() too for needed checks
const u16 ML = 0x10;  // set SE- and AOS-Flag in send_feature_enable() too for needed checks
const u16 KR =
    0x20;  // set KR- and ML- and SE- and AOS-Flag in send_feature_enable() too for needed checks
const u16 SA =
    0x40;  // set SA- and KR- and SE- and AOS-Flag in send_feature_enable() too for needed checks
const u16 HSA = 0x80;  // set HSA- and SA- and KR- and SE- and AOS-Flag in send_feature_enable() too
                       // for needed checks
const u16 TOL = 0x100;  // set TOL- and HSA- and SA- and KR- and SE- and AOS-Flag in
                        // send_feature_enable() too for needed checks

const u8 FLAG_GENDER = 0x01;
const u8 FLAG_RACE = 0x02;

struct VersionDetailStruct
{
  int major;
  int minor;
  int rev;
  int patch;
};

const struct VersionDetailStruct CLIENT_VER_4000 = {4, 0, 0, 0};
const struct VersionDetailStruct CLIENT_VER_4070 = {4, 0, 7, 0};
const struct VersionDetailStruct CLIENT_VER_5000 = {5, 0, 0, 0};
const struct VersionDetailStruct CLIENT_VER_5020 = {5, 0, 2, 0};
const struct VersionDetailStruct CLIENT_VER_6017 = {6, 0, 1, 7};
const struct VersionDetailStruct CLIENT_VER_60142 = {6, 0, 14, 2};
const struct VersionDetailStruct CLIENT_VER_7000 = {7, 0, 0, 0};
const struct VersionDetailStruct CLIENT_VER_7090 = {7, 0, 9, 0};
const struct VersionDetailStruct CLIENT_VER_70130 = {7, 0, 13, 0};
const struct VersionDetailStruct CLIENT_VER_70300 = {7, 0, 30, 0};
const struct VersionDetailStruct CLIENT_VER_70331 = {7, 0, 33, 1};

enum ClientTypeFlag
{
  CLIENTTYPE_4000 = 0x1,    // 4.0.0a   (new spellbookcontent packet 0xbf:0x1b)
  CLIENTTYPE_4070 = 0x2,    // 4.0.7a   (new damage packet 0x0b instead of 0xbf:0x22)
  CLIENTTYPE_5000 = 0x4,    // 5.0.0a   (compressed gumps)
  CLIENTTYPE_5020 = 0x8,    // 5.0.2a   (Buff/Debuff 0xdf)
  CLIENTTYPE_6017 = 0x10,   // 6.0.1.7  (Grid locs)
  CLIENTTYPE_60142 = 0x20,  // 6.0.14.2 (feature enable 0xb9 size change)
  CLIENTTYPE_UOKR = 0x40,
  CLIENTTYPE_7000 = 0x80,  // 7.0.0.0  (Gargoyle race)
  CLIENTTYPE_UOSA = 0x100,
  CLIENTTYPE_7090 = 0x200,   // 7.0.9.0 (High Sea Adventures)
  CLIENTTYPE_70130 = 0x400,  // 7.0.13.0 (New 0xA9 packet)
  CLIENTTYPE_70300 = 0x800,  // 7.0.30.0 (New Status entries for classic client)
  CLIENTTYPE_70331 = 0x1000  // 7.0.33.1 new mobile incoming
};

typedef struct
{
  unsigned char pktbuffer[PKTIN_02_SIZE];
} PacketThrottler;

class Client
{
public:
  Client( ClientInterface& aInterface, Crypt::TCryptInfo& encryption );
  Client( const Client& ) = delete;
  Client& operator=( const Client& ) = delete;
  static void Delete( Client* client );
  size_t estimatedSize() const;

private:
  void PreDelete();
  ~Client();
  bool preDisconnect;
  bool disconnect;  // if 1, disconnect this client

public:
  void Disconnect();
  void forceDisconnect();
  bool isActive() const;
  bool isReallyConnected() const;
  bool isConnected() const;

  void unregister();  // removes updater for vitals and takes client away from clientlist
  void closeConnection();
  void transmit( const void* data, int len ); // always obtains PolLock when calling a SendFunction

  void recv_remaining( int total_expected );
  void recv_remaining_nocrypt( int total_expected );

  void setversion( const std::string& ver ) { version_ = ver; }
  const std::string& getversion() const { return version_; }
  VersionDetailStruct getversiondetail() const { return versiondetail_; }
  void setversiondetail( VersionDetailStruct& detail ) { versiondetail_ = detail; }
  static void itemizeclientversion( const std::string& ver, VersionDetailStruct& detail );
  bool compareVersion( const std::string& ver );
  bool compareVersion( const VersionDetailStruct& ver2 );
  void setClientType( ClientTypeFlag type );
  bool IsUOKRClient();

  // Tells whether we should use a different packet handler for this client version
  bool might_use_v2_handler() const;

  void setclientinfo( const Core::PKTIN_D9* msg )
  {
    memcpy( &clientinfo_, msg, sizeof( clientinfo_ ) );
  }
  Bscript::BStruct* getclientinfo() const;

  Accounts::Account* acct;
  Mobile::Character* chr;
  ClientInterface& Interface;

  bool ready;  // all initialization stuff has been sent, ready for general use.


  //
  bool have_queued_data() const;
  void send_queued_data();

  SOCKET csocket;  // socket to client ACK  - requires header inclusion.
  static std::mutex _SocketMutex;
  unsigned short listen_port;
  bool aosresist;  // UOClient.Cfg Entry


  enum e_recv_states
  {
    RECV_STATE_CRYPTSEED_WAIT,
    RECV_STATE_MSGTYPE_WAIT,
    RECV_STATE_MSGLEN_WAIT,
    RECV_STATE_MSGDATA_WAIT,
    RECV_STATE_CLIENTVERSION_WAIT
  } recv_state;

  unsigned char bufcheck1_AA;
  unsigned char buffer[MAXBUFFER];
  unsigned char bufcheck2_55;
  unsigned int bytes_received;  // how many bytes have been received into the buffer.
  unsigned int message_length;  // how many bytes are expected for this message

  sockaddr ipaddr;

  Crypt::CCryptBase* cryptengine;

  bool encrypt_server_stream;  // encrypt the server stream (data sent to client)?

  const Core::MessageTypeFilter* msgtype_filter;

  mutable Clib::SpinLock _fpLog_lock;
  std::string fpLog;

  std::string status() const;

  void send_pause();
  void send_restart();

  void pause();
  void restart();
  std::atomic<int> pause_count;

  std::string ipaddrAsString() const;

  bool SpeedHackPrevention( bool add = true );
  Bscript::BObjectImp* make_ref();
  weak_ptr<Client> getWeakPtr() const;

protected:
  Core::XmitBuffer* first_xmit_buffer;
  Core::XmitBuffer* last_xmit_buffer;
  int n_queued;
  int queued_bytes_counter;  // only used for monitoring

  // we may want to track how many bytes total are outstanding,
  // and boot clients that are too far behind.
  void queue_data( const void* data, unsigned short datalen );
  void transmit_encrypted( const void* data, int len );
  void xmit( const void* data, unsigned short datalen );

public:
  ClientGameData* gd;
  unsigned int instance_;
  static unsigned int instance_counter_;
  int checkpoint;  // CNXBUG
  unsigned char last_msgtype;
  size_t thread_pid;
  u16 UOExpansionFlag;
  u32 UOExpansionFlagClient;
  u16 ClientType;
  std::queue<PacketThrottler> movementqueue;
  Clib::wallclock_t next_movement;
  u8 movementsequence;
  // Will be set by clientthread
  std::atomic<Core::polclock_t> last_activity_at;
  std::atomic<Core::polclock_t> last_packet_at;

private:
  struct
  {
    unsigned int bytes_transmitted;
    unsigned int bytes_received;
  } counters;
  std::string version_;
  Core::PKTIN_D9 clientinfo_;
  bool paused_;
  VersionDetailStruct versiondetail_;
  weak_ptr_owner<Client> weakptr;
};

inline bool Client::have_queued_data() const
{
  return ( first_xmit_buffer != nullptr );
}


// Disconnects client. Might lose packets that were not sent by packetqueue.
inline void Client::forceDisconnect()
{
  this->disconnect = true;
  closeConnection();
}

// Checks whether the client is disconnected, and not only marked for disconnection
inline bool Client::isReallyConnected() const
{
  return !this->disconnect && this->csocket != INVALID_SOCKET;
}

// Checks for both planned and executed disconnections
inline bool Client::isConnected() const
{
  return !this->preDisconnect && this->isReallyConnected();
}

inline bool Client::isActive() const
{
  return this->ready && this->isConnected();
}

inline bool Client::might_use_v2_handler() const
{
  return ( this->ClientType & Network::CLIENTTYPE_6017 ) != 0;
}
}  // namespace Network
}  // namespace Pol
#endif
