/** @file
 *
 * @par History
 * - 2005/01/24 Shinigami: added getspyonclient2 to support packet 0xd9 (Spy on Client 2)
 * - 2005/08/29 Shinigami: character.spyonclient2 renamed to character.clientinfo
 *                         getspyonclient2 renamed to getclientinfo
 * - 2007/07/09 Shinigami: added isUOKR [bool] - UO:KR client used?
 * - 2009/07/20 MuadDib:   Added statement to bypass cryptseed at login. Handled by changing default
 * client recv_state using ssopt flag.
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of aosresist
 *                         STLport-5.2.1 fix: params in call of Log2()
 * - 2009/09/06 Turley:    Added u8 ClientType + FlagEnum
 *                         Removed is*
 * - 2010/01/22 Turley:    Speedhack Prevention System
 */


#include "client.h"

#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "../../bscript/berror.h"
#include "../../bscript/bstruct.h"
#include "../../bscript/impstr.h"
#include "../../clib/clib.h"
#include "../../clib/logfacility.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"  //CNXBUG
#include "../../clib/wallclock.h"
#include "../accounts/account.h"
#include "../crypt/cryptbase.h"
#include "../crypt/cryptengine.h"
#include "../globals/network.h"
#include "../globals/settings.h"
#include "../globals/state.h"
#include "../globals/uvars.h"
#include "../mobile/charactr.h"
#include "../polsig.h"
#include "../realms/WorldChangeReasons.h"
#include "../ufunc.h"  // only in here temporarily, until logout-on-disconnect stuff is removed
#include "../uoclient.h"
#include "../uoscrobj.h"
#include "../uworld.h"
#include "cgdata.h"
#include "cliface.h"
#include "packethelper.h"
#include "packets.h"
#include "pktdef.h"
#include "pktin.h"
#include "xbuffer.h"


#define PRE_ENCRYPT

#ifndef PRE_ENCRYPT
#include "sockio.h"
#endif

#ifdef _MSC_VER
#pragma warning( \
    disable : 4351 )  // new behavior: elements of array '...' will be default initialized
#endif

namespace Pol
{
namespace Core
{
void cancel_trade( Mobile::Character* chr1 );
}
namespace Network
{
unsigned int Client::instance_counter_;

ThreadedClient::ThreadedClient( Crypt::TCryptInfo& encryption, Client& myClient,
                                sockaddr& client_addr,
                                std::vector<boost::asio::ip::network_v4>& allowed_proxies )
    : myClient( myClient ),
      thread_pid( static_cast<size_t>( -1 ) ),
      csocket( INVALID_SOCKET ),
      preDisconnect( false ),
      disconnect( false ),
      cryptengine( create_crypt_engine( encryption ) ),
      encrypt_server_stream( false ),
      allowed_proxies( allowed_proxies ),
      last_activity_at( 0 ),
      last_packet_at( 0 ),
      recv_state( RECV_STATE_CRYPTSEED_WAIT ),
      bufcheck1_AA( 0xAA ),
      buffer(),  // zero-initializes the buffer
      bufcheck2_55( 0x55 ),
      bytes_received( 0 ),
      message_length( 0 ),
      last_msgtype( 255 ),
      msgtype_filter( Core::networkManager.login_filter.get() ),
      checkpoint( -1 ),  // CNXBUG
      _fpLog_lock(),
      fpLog( "" ),
      disable_inactivity_timeout( false ),
      first_xmit_buffer( nullptr ),
      last_xmit_buffer( nullptr ),
      n_queued( 0 ),
      queued_bytes_counter( 0 )
{
  memset( &counters, 0, sizeof counters );
  memcpy( &ipaddr, &client_addr, sizeof ipaddr );
  memset( &ipaddr_proxy, 0, sizeof( ipaddr_proxy ) );

  if ( ipaddr.sa_family == AF_INET )
  {
    // accept proxy protocol only from allowed ips
    auto ipaddrv4 = reinterpret_cast<sockaddr_in*>( &ipaddr );
    auto my_address =
#ifdef _WIN32
        boost::asio::ip::address_v4( htonl( ipaddrv4->sin_addr.S_un.S_addr ) );
#else
        boost::asio::ip::address_v4( htonl( ipaddrv4->sin_addr.s_addr ) );
#endif
    auto my_network = boost::asio::ip::network_v4( my_address, 32 );
    for ( const auto& allowed_proxy : allowed_proxies )
    {
      if ( my_network == allowed_proxy || my_network.is_subnet_of( allowed_proxy ) )
      {
        recv_state = RECV_STATE_PROXYPROTOCOLHEADER_WAIT;
        break;
      }
    }
  }
};

Client::Client( ClientInterface& aInterface, Crypt::TCryptInfo& encryption, sockaddr& ipaddr,
                std::vector<boost::asio::ip::network_v4>& allowed_proxies )
    : ThreadedClient( encryption, *this, ipaddr, allowed_proxies ),
      acct( nullptr ),
      chr( nullptr ),
      Interface( aInterface ),
      ready( false ),
      listen_port( 0 ),
      aosresist( false ),
      pause_count( 0 ),
      gd( new ClientGameData ),
      instance_( ++instance_counter_ ),
      UOExpansionFlagClient( 0 ),
      ClientType( 0 ),
      next_movement( 0 ),
      movementsequence( 0 ),
      paused_( false )
{
  weakptr.set( this );  // store weakptr for usage in scripts (see EClientRefObjImp)

  // For bypassing cryptseed packet
  if ( Core::settingsManager.ssopt.use_edit_server )
  {
    recv_state = RECV_STATE_MSGTYPE_WAIT;
  }

  Interface.register_client( this );

  memset( &clientinfo_, 0, sizeof( clientinfo_ ) );
  memset( &versiondetail_, 0, sizeof( versiondetail_ ) );
}

Client::~Client()
{
  PreDelete();
  delete cryptengine;
}

void Client::init_crypto( void* nseed, int type )
{
  session()->cryptengine->Init( nseed, type );
}

void Client::unregister()
{
  auto findClient =
      std::find( Core::networkManager.clients.begin(), Core::networkManager.clients.end(), this );
  Core::networkManager.clients.erase( findClient );  // TODO: Make networkManager more OO
  Interface.deregister_client( this );
}

void Client::PreDelete()
{
  closeConnection();

  if ( chr != nullptr && chr->client == this )
  {
    if ( chr->logged_in() )
    {
      ClrCharacterWorldPosition( chr, Realms::WorldChangeReason::PlayerExit );
      send_remove_character_to_nearby( chr );
      chr->logged_in( false );

      chr->set_opponent( nullptr );
      chr->removal_cleanup();
      if ( chr->get_opponent() != nullptr )
      {
        chr->set_opponent( nullptr, true );
      }
    }
    else
    {
      ERROR_PRINTLN( "Uhh...  active character not logged in!??" );
    }
  }

  // detach the account and character from this client, if they
  // are still associated with it.

  acct = nullptr;

  if ( chr )
  {
    if ( chr->client == this )
      chr->client = nullptr;
    chr = nullptr;
  }

  // stop packet-logging
  stop_log();

  delete gd;
  gd = nullptr;

  while ( first_xmit_buffer != nullptr )
  {
    Core::XmitBuffer* xbuffer = first_xmit_buffer;
    first_xmit_buffer = first_xmit_buffer->next;
    free( xbuffer );
    --n_queued;
  }
  last_xmit_buffer = nullptr;

  // while (!movementqueue.empty())
  //  movementqueue.pop();
}

// ClientInfo - delivers a lot of usefull infomation about client PC
Bscript::BStruct* Client::getclientinfo() const
{
  using namespace Bscript;
  std::unique_ptr<BStruct> ret( new BStruct );

  ret->addMember( "unknown1", new BLong( clientinfo_.unknown1 ) );  // Unknown - allways 0x02
  ret->addMember( "instance", new BLong( clientinfo_.instance ) );  // Unique Instance ID of UO
  ret->addMember( "os_major", new BLong( clientinfo_.os_major ) );  // OS Major
  ret->addMember( "os_minor", new BLong( clientinfo_.os_minor ) );  // OS Minor
  ret->addMember( "os_revision", new BLong( clientinfo_.os_revision ) );  // OS Revision
  ret->addMember( "cpu_manufacturer",
                  new BLong( clientinfo_.cpu_manufacturer ) );          // CPU Manufacturer
  ret->addMember( "cpu_family", new BLong( clientinfo_.cpu_family ) );  // CPU Family
  ret->addMember( "cpu_model", new BLong( clientinfo_.cpu_model ) );    // CPU Model
  ret->addMember( "cpu_clockspeed",
                  new BLong( clientinfo_.cpu_clockspeed ) );                // CPU Clock Speed [Mhz]
  ret->addMember( "cpu_quantity", new BLong( clientinfo_.cpu_quantity ) );  // CPU Quantity
  ret->addMember( "memory", new BLong( clientinfo_.memory ) );              // Memory [MB]
  ret->addMember( "screen_width", new BLong( clientinfo_.screen_width ) );  // Screen Width
  ret->addMember( "screen_height", new BLong( clientinfo_.screen_height ) );  // Screen Height
  ret->addMember( "screen_depth", new BLong( clientinfo_.screen_depth ) );    // Screen Depth [Bit]
  ret->addMember( "directx_major", new BLong( clientinfo_.directx_major ) );  // DirectX Major
  ret->addMember( "directx_minor", new BLong( clientinfo_.directx_minor ) );  // DirectX Minor

  unsigned maxlen_vd =
      sizeof( clientinfo_.video_description ) / sizeof( clientinfo_.video_description[0] );
  std::string vd = Bscript::String::fromUTF16( &clientinfo_.video_description[0], maxlen_vd, true );
  ret->addMember( "video_description",
                  new Bscript::String( vd ) );  // Video Card Description


  ret->addMember( "video_vendor", new BLong( clientinfo_.video_vendor ) );  // Video Card Vendor ID
  ret->addMember( "video_device", new BLong( clientinfo_.video_device ) );  // Video Card Device ID
  ret->addMember( "video_memory",
                  new BLong( clientinfo_.video_memory ) );  // Video Card Memory [MB]
  ret->addMember( "distribution", new BLong( clientinfo_.distribution ) );        // Distribution
  ret->addMember( "clients_running", new BLong( clientinfo_.clients_running ) );  // Clients Running
  ret->addMember( "clients_installed",
                  new BLong( clientinfo_.clients_installed ) );  // Clients Installed
  ret->addMember( "partial_installed",
                  new BLong( clientinfo_.partial_installed ) );  // Partial Insstalled

  unsigned maxlen_lc = sizeof( clientinfo_.langcode ) / sizeof( clientinfo_.langcode[0] );
  std::string lc = Bscript::String::fromUTF16( &clientinfo_.langcode[0], maxlen_lc, true );
  ret->addMember( "langcode",
                  new Bscript::String( lc ) );  // Language Code

  std::unique_ptr<ObjArray> arr_u2( new ObjArray );
  for ( unsigned i = 0; i < sizeof( clientinfo_.unknown2 ); ++i )
    arr_u2->addElement( new BLong( clientinfo_.unknown2[i] ) );
  ret->addMember( "unknown2", arr_u2.release() );  // Unknown

  return ret.release();
}

void Client::itemizeclientversion( const std::string& ver, VersionDetailStruct& detail )
{
  try
  {
    size_t dot1 = ver.find_first_of( '.', 0 );
    size_t dot2 = ver.find_first_of( '.', dot1 + 1 );
    size_t dot3 = ver.find_first_of( '.', dot2 + 1 );
    if ( dot3 == std::string::npos )  // since 5.0.7 patch is digit
    {
      dot3 = dot2 + 1;
      while ( ( dot3 < ver.length() ) && ( isdigit( ver[dot3] ) ) )
      {
        dot3++;
      }
    }

    detail.major = atoi( ver.substr( 0, dot1 ).c_str() );
    detail.minor = atoi( ver.substr( dot1 + 1, dot2 - dot1 - 1 ).c_str() );
    detail.rev = atoi( ver.substr( dot2 + 1, dot3 - dot2 - 1 ).c_str() );
    detail.patch = 0;
    if ( dot3 < ver.length() )
    {
      if ( ( detail.major <= 5 ) && ( detail.minor <= 0 ) && ( detail.rev <= 6 ) )
      {
        if ( ver[dot3] != ' ' )
          detail.patch = ( ver[dot3] - 'a' ) + 1;  // char to int
      }
      else
        detail.patch = atoi( ver.substr( dot3 + 1, ver.length() - dot3 - 1 ).c_str() );
    }
  }
  catch ( ... )
  {
    detail.major = 0;
    detail.minor = 0;
    detail.rev = 0;
    detail.patch = 0;
    POLLOGLN( "Malformed clientversion string: {}", ver );
  }
}

bool Client::compareVersion( const std::string& ver )
{
  VersionDetailStruct ver2;
  itemizeclientversion( ver, ver2 );
  return Client::compareVersion( ver2 );
}

bool Client::compareVersion( const VersionDetailStruct& ver2 )
{
  VersionDetailStruct ver1 = getversiondetail();

  if ( ver1.major > ver2.major )
    return true;
  else if ( ver1.major < ver2.major )
    return false;
  else if ( ver1.minor > ver2.minor )
    return true;
  else if ( ver1.minor < ver2.minor )
    return false;
  else if ( ver1.rev > ver2.rev )
    return true;
  else if ( ver1.rev < ver2.rev )
    return false;
  else if ( ver1.patch > ver2.patch )
    return true;
  else if ( ver1.patch < ver2.patch )
    return false;
  else
    return true;
}

void Client::setClientType( ClientTypeFlag type )
{
  ClientType = 0x0;
  // with fall through !
  switch ( type )
  {
  case CLIENTTYPE_70331:
    ClientType |= CLIENTTYPE_70331;
  // fall through
  case CLIENTTYPE_70300:
    ClientType |= CLIENTTYPE_70300;
  // fall through
  case CLIENTTYPE_70130:
    ClientType |= CLIENTTYPE_70130;
  // fall through
  case CLIENTTYPE_7090:
    ClientType |= CLIENTTYPE_7090;
  // fall through
  case CLIENTTYPE_UOSA:
    ClientType |= CLIENTTYPE_UOSA;
  // fall through
  case CLIENTTYPE_7000:
    ClientType |= CLIENTTYPE_7000;
  // fall through
  case CLIENTTYPE_UOKR:
    ClientType |= CLIENTTYPE_UOKR;
  // fall through
  case CLIENTTYPE_60142:
    ClientType |= CLIENTTYPE_60142;
  // fall through
  case CLIENTTYPE_6017:
    ClientType |= CLIENTTYPE_6017;
  // fall through
  case CLIENTTYPE_5020:
    ClientType |= CLIENTTYPE_5020;
  // fall through
  case CLIENTTYPE_5000:
    ClientType |= CLIENTTYPE_5000;
  // fall through
  case CLIENTTYPE_4070:
    ClientType |= CLIENTTYPE_4070;
  // fall through
  case CLIENTTYPE_4000:
    ClientType |= CLIENTTYPE_4000;
  // fall through
  default:
    break;
  }
}

bool Client::IsUOKRClient()
{
  return ( ( ClientType & CLIENTTYPE_UOKR ) && ( !( ClientType & CLIENTTYPE_7000 ) ) );
}

std::string Client::status() const
{
  std::string st;
  if ( acct != nullptr )
    st += "AC:" + std::string( acct->name() ) + " ";
  if ( chr != nullptr )
    st += "CH:" + chr->name() + " ";
  if ( have_queued_data() )
    st += "TXBUF ";
  if ( disconnect )
    st += "DISC ";
  if ( paused_ )
    st += "PAUSE ";
  if ( ready )
    st += "RDY ";
  st += ipaddrAsString() + " ";
  st += "CHK: " + Clib::tostring( checkpoint ) + " ";
  st += "PID: " + Clib::tostring( thread_pid ) + " ";
  st += "LAST: " + Clib::hexint( last_msgtype );
  return st;
}

void ThreadedClient::queue_data( const void* data, unsigned short datalen )
{
  THREAD_CHECKPOINT( active_client, 300 );
  Core::XmitBuffer* xbuffer = (Core::XmitBuffer*)malloc( sizeof( Core::XmitBuffer ) - 1 + datalen );
  THREAD_CHECKPOINT( active_client, 301 );
  if ( xbuffer )
  {
    THREAD_CHECKPOINT( active_client, 302 );
    xbuffer->next = nullptr;
    xbuffer->nsent = 0;
    xbuffer->lenleft = datalen;
    memcpy( xbuffer->data, data, datalen );
    THREAD_CHECKPOINT( active_client, 303 );
    if ( first_xmit_buffer == nullptr || last_xmit_buffer == nullptr )
    {  // in this case, last_xmit_buffer is also nullptr, so can't set its ->next.
      THREAD_CHECKPOINT( active_client, 304 );
      first_xmit_buffer = xbuffer;
    }
    else
    {
      THREAD_CHECKPOINT( active_client, 305 );
      last_xmit_buffer->next = xbuffer;
    }
    THREAD_CHECKPOINT( active_client, 306 );
    last_xmit_buffer = xbuffer;
    ++n_queued;
    queued_bytes_counter += datalen;
  }
  else
  {
    THREAD_CHECKPOINT( active_client, 307 );
    POLLOGLN( "Client#{}: Unable to allocate {} bytes for queued data.  Disconnecting.",
              myClient.instance_, ( sizeof( Core::XmitBuffer ) - 1 + datalen ) );
    disconnect = true;
  }
  THREAD_CHECKPOINT( active_client, 309 );
}

void ThreadedClient::xmit( const void* data, unsigned short datalen )
{
  if ( csocket == INVALID_SOCKET )
    return;
  if ( encrypt_server_stream )
  {
    if ( cryptengine == nullptr )
      return;
    this->cryptengine->Encrypt( (void*)data, (void*)data, datalen );
  }
  THREAD_CHECKPOINT( active_client, 200 );
  if ( last_xmit_buffer )  // this client already backlogged, schedule for later
  {
    THREAD_CHECKPOINT( active_client, 201 );
    queue_data( data, datalen );
    THREAD_CHECKPOINT( active_client, 202 );
    return;
  }
  THREAD_CHECKPOINT( active_client, 203 );

  /* client not backlogged - try to send. */
  const unsigned char* cdata = (const unsigned char*)data;
  int nsent;

  if ( -1 == ( nsent = send( csocket, (const char*)cdata, datalen, 0 ) ) )
  {
    THREAD_CHECKPOINT( active_client, 204 );
    int sckerr = socket_errno;

    if ( sckerr == SOCKET_ERRNO( EWOULDBLOCK ) )
    {
      THREAD_CHECKPOINT( active_client, 205 );
      POLLOG_ERRORLN( "Client#{}: Switching to queued data mode (1, {} bytes)", myClient.instance_,
                      datalen );
      THREAD_CHECKPOINT( active_client, 206 );
      queue_data( data, datalen );
      THREAD_CHECKPOINT( active_client, 207 );
      return;
    }
    else
    {
      THREAD_CHECKPOINT( active_client, 208 );
      if ( !disconnect )
        POLLOG_ERRORLN( "Client#{}: Disconnecting client due to send() error (1): {}",
                        myClient.instance_, sckerr );
      disconnect = true;
      THREAD_CHECKPOINT( active_client, 209 );
      return;
    }
  }
  else  // no error
  {
    THREAD_CHECKPOINT( active_client, 210 );
    datalen -= static_cast<unsigned short>( nsent );
    counters.bytes_transmitted += nsent;
    Core::networkManager.polstats.bytes_sent += nsent;
    if ( datalen )  // anything left? if so, queue for later.
    {
      THREAD_CHECKPOINT( active_client, 211 );
      POLLOG_ERRORLN( "Client#{}: Switching to queued data mode (2)", myClient.instance_ );
      THREAD_CHECKPOINT( active_client, 212 );
      queue_data( cdata + nsent, datalen );
      THREAD_CHECKPOINT( active_client, 213 );
    }
  }
  THREAD_CHECKPOINT( active_client, 214 );
}

void ThreadedClient::send_queued_data()
{
  std::lock_guard<std::mutex> lock( _socketMutex );
  Core::XmitBuffer* xbuffer;
  // hand off data to the sockets layer until it won't take any more.
  // note if a buffer is sent in full, we try to send the next one, ad infinitum
  while ( nullptr != ( xbuffer = first_xmit_buffer ) )
  {
    int nsent;
    nsent = send( csocket, (char*)&xbuffer->data[xbuffer->nsent], xbuffer->lenleft, 0 );
    if ( nsent == -1 )
    {
#ifdef _WIN32
      int sckerr = WSAGetLastError();
#else
      int sckerr = errno;
#endif
      if ( sckerr == SOCKET_ERRNO( EWOULDBLOCK ) )
      {
        // do nothing.  it'll be re-queued later, when it won't block.
        return;
      }
      else
      {
        if ( !disconnect )
          POLLOGLN( "Client#{}: Disconnecting client due to send() error (2): {}",
                    myClient.instance_, sckerr );
        disconnect = true;
        return;
      }
    }
    else
    {
      xbuffer->nsent += static_cast<unsigned short>( nsent );
      xbuffer->lenleft -= static_cast<unsigned short>( nsent );
      counters.bytes_transmitted += nsent;
      Core::networkManager.polstats.bytes_sent += nsent;
      if ( xbuffer->lenleft == 0 )
      {
        first_xmit_buffer = first_xmit_buffer->next;
        if ( first_xmit_buffer == nullptr )
        {
          last_xmit_buffer = nullptr;
          POLLOGLN( "Client#{}: Leaving queued mode ({} bytes xmitted)", myClient.instance_,
                    queued_bytes_counter );
          queued_bytes_counter = 0;
        }
        free( xbuffer );
        --n_queued;
      }
    }
  }
}

// 33 01 "encrypted": 4F FA
static const unsigned char pause_pre_encrypted[2] = { 0x4F, 0xFA };
// 33 00 "encrypted": 4C D0
static const unsigned char restart_pre_encrypted[2] = { 0x4C, 0xD0 };

void Client::send_pause()
{
  if ( Core::networkManager.uoclient_protocol.EnableFlowControlPackets && !paused_ )
  {
#ifndef PRE_ENCRYPT
    PKTOUT_33 msg;
    msg.msgtype = PKTOUT_33_ID;
    msg.flow = MSGOPT_33_FLOW_PAUSE;
    transmit( this, &msg, sizeof msg );
#else
    xmit( pause_pre_encrypted, sizeof pause_pre_encrypted );
#endif
    paused_ = true;
  }
}

void Client::pause()
{
  if ( !pause_count )
  {
    send_pause();
    pause_count = 1;
  }
}

void Client::send_restart()
{
  if ( paused_ )
  {
#ifndef PRE_ENCRYPT
    PKTOUT_33 msg;
    msg.msgtype = PKTOUT_33_ID;
    msg.flow = MSGOPT_33_FLOW_RESTART;
    transmit( this, &msg, sizeof msg );
#else
    xmit( restart_pre_encrypted, sizeof restart_pre_encrypted );
#endif
    paused_ = false;
  }
}

void Client::restart()
{
  send_restart();
  pause_count = 0;
}

// Note: this doesnt test single packets it only summs the delay and tests
// here only the "start"-value is set the additional delay is set in PKT_02 handler
bool Client::SpeedHackPrevention( bool add )
{
  if ( ( !movementqueue.empty() ) && ( add ) )
  {
    if ( movementqueue.size() > 100 )
    {
      POLLOG_ERRORLN( "Client#{}: More then 100 Movepackets in queue.  Disconnecting.", instance_ );
      disconnect = true;
      return false;
    }
    PacketThrottler throttlestruct;
    memcpy( &throttlestruct.pktbuffer, &buffer, PKTIN_02_SIZE );
    movementqueue.push( throttlestruct );
    return false;
  }
  if ( chr != nullptr && chr->can_speedhack() )
    return true;
  if ( ( next_movement == 0 ) ||
       ( Clib::wallclock() > next_movement ) )  // never moved or in the past
  {
    next_movement = Clib::wallclock();
    return true;
  }
  // now we dont alter next_movement so we can sum the delay till diff is greater then error margin
  Clib::wallclock_diff_t diff = Clib::wallclock_diff_ms( Clib::wallclock(), next_movement );
  if ( diff > PKTIN_02_ASYNCHRONOUS )  // delay sum greater then our error margin?
  {
    if ( add )  // delay packet
    {
      if ( movementqueue.size() > 100 )
      {
        POLLOG_ERRORLN( "Client#{}: More then 100 Movepackets in queue.  Disconnecting.",
                        instance_ );
        disconnect = true;
        return false;
      }
      PacketThrottler throttlestruct;
      memcpy( &throttlestruct.pktbuffer, &buffer, sizeof( throttlestruct.pktbuffer ) );
      movementqueue.push( throttlestruct );
    }
    return false;
  }
  return true;
}

Bscript::BObjectImp* Client::make_ref()
{
  return new Module::EClientRefObjImp( weakptr );
}

weak_ptr<Client> Client::getWeakPtr() const
{
  return weakptr;
}
void Client::set_update_range_by_client( u8 range )
{
  // only allow a change if allowed and not modified by script
  if ( Core::settingsManager.ssopt.allow_visual_range_modification )
  {
    // limit range to defined min/max
    range = std::clamp( range, Core::settingsManager.ssopt.min_visual_range,
                        Core::settingsManager.ssopt.max_visual_range );
    if ( !gd->script_defined_update_range )
      set_update_range( range );
    gd->original_client_update_range = range;
  }
  else
  {
    PktHelper::PacketOut<Network::PktOut_C8> outMsg;
    outMsg->Write<u8>( update_range() );
    outMsg.Send( this );
    gd->original_client_update_range = Core::settingsManager.ssopt.default_visual_range;
  }
}

void Client::set_update_range_by_script( u8 range )
{
  if ( range == 0 )
  {
    range = gd->original_client_update_range ? gd->original_client_update_range
                                             : Core::settingsManager.ssopt.default_visual_range;
    gd->script_defined_update_range = false;
  }
  else
  {
    gd->script_defined_update_range = true;
  }
  // no limit check here, script is allowed to use different values
  set_update_range( range );
}

void Client::set_update_range( u8 range )
{
  auto old_range = update_range();
  // first signal the client about the change before we potentially send new objects otherwise the
  // client could directly drop them
  PktHelper::PacketOut<PktOut_C8> outMsg;
  outMsg->Write<u8>( range );
  outMsg.Send( this );

  // delete/send all objects, but not if its the first pkt
  if ( old_range != range && gd->original_client_update_range != 0 )
    chr->update_objects_on_range_change( range );

  gd->update_range = range;

  if ( old_range != range )
  {
    // update global updaterange (maximum multi radius/client view range)
    Core::gamestate.update_range_from_client( range );
  }
}

u8 Client::update_range() const
{
  return gd->update_range;
}

bool Client::acctSupports( Plib::ExpansionVersion v ) const
{
  return acct->expansion().hasExpansion( v );
}

size_t Client::estimatedSize() const
{
  Clib::SpinLockGuard guard( _fpLog_lock );
  size_t size = ThreadedClient::estimatedSize();
  size += sizeof( void* ) * 3                              /*acct, chr, Interface*/
          + sizeof( bool )                                 /* ready */
          + sizeof( unsigned short )                       /* listenpoint */
          + sizeof( bool )                                 /* aosresist */
          + sizeof( std::atomic<int> )                     /* pause_count */
          + sizeof( void* )                                /* gd */
          + sizeof( unsigned int )                         /* instance_ */
          + sizeof( u32 )                                  /* UOExpansionFlagClient */
          + sizeof( u16 )                                  /* ClientType */
          + sizeof( Clib::wallclock_t )                    /* next_movement */
          + sizeof( u8 )                                   /* movementsequence */
          + version_.capacity() + sizeof( Core::PKTIN_D9 ) /* clientinfo_*/
          + sizeof( bool )                                 /* paused_ */
          + sizeof( VersionDetailStruct )                  /* versiondetail_ */
          + sizeof( weak_ptr_owner<Client> )               /*weakptr*/
      ;
  size += Clib::memsize( movementqueue );
  if ( gd != nullptr )
    size += gd->estimatedSize();
  return size;
}

// Threaded client stuff

size_t ThreadedClient::estimatedSize() const
{
  size_t size = sizeof( ThreadedClient ) + Clib::memsize( allowed_proxies ) + fpLog.capacity();
  Core::XmitBuffer* buffer_size = first_xmit_buffer;
  while ( buffer_size != nullptr )
  {
    size += sizeof( buffer_size ) + buffer_size->lenleft;
    buffer_size = buffer_size->next;
  }
  return size;
}
void ThreadedClient::closeConnection()
{
  std::lock_guard<std::mutex> lock( _socketMutex );
  if ( csocket != INVALID_SOCKET )
  {
#ifdef _WIN32
    shutdown( csocket, 2 );  // 2 is both sides, defined in winsock2.h ...
    closesocket( csocket );
#else
    shutdown( csocket, SHUT_RDWR );
    close( csocket );
#endif
  }
  csocket = INVALID_SOCKET;
}


}  // namespace Network
}  // namespace Pol
