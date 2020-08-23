/** @file
 *
 * @par History
 * - 2009/12/04 Turley:    Crypto cleanup - Tomi
 */


#include <errno.h>
#include <mutex>
#include <stddef.h>
#include <string>

#include "../../clib/fdump.h"
#include "../../clib/logfacility.h"
#include "../../clib/network/sockets.h"
#include "../../clib/passert.h"
#include "../../clib/refptr.h"
#include "../../clib/spinlock.h"
#include "../crypt/cryptbase.h"
#include "../ctable.h"
#include "../globals/network.h"
#include "../globals/state.h"
#include "../packetscrobj.h"
#include "../polsem.h"
#include "../polsig.h"
#include "client.h"
#include "clienttransmit.h"
#include "packethelper.h"
#include "packethooks.h"
#include "packets.h"
#include <format/format.h>

namespace Pol
{
namespace Network
{
std::string Client::ipaddrAsString() const
{
  return AddressToString( &this->ipaddr );
}

void Client::recv_remaining( int total_expected )
{
  int count;
  int max_expected = total_expected - bytes_received;

  {
    std::lock_guard<std::mutex> lock( client_socket_mutex ); // unnecessary?
    count = cryptengine->Receive( &buffer[bytes_received], max_expected, csocket );
  }

  if ( count > 0 )
  {
    passert( count <= max_expected );

    bytes_received += count;
    counters.bytes_received += count;
    Core::networkManager.polstats.bytes_received += count;
  }
  else if ( count == 0 )  // graceful close
  {
    disconnect = true;
  }
  else
  {
    int errn = socket_errno;
    if ( errn != SOCKET_ERRNO( EWOULDBLOCK ) )
      disconnect = true;
  }
}

void Client::recv_remaining_nocrypt( int total_expected )
{
  int count;

  {
    std::lock_guard<std::mutex> lock( client_socket_mutex ); // unnecessary?
    count = recv( csocket, (char*)&buffer[bytes_received], total_expected - bytes_received, 0 );
  }
  if ( count > 0 )
  {
    bytes_received += count;
    counters.bytes_received += count;
    Core::networkManager.polstats.bytes_received += count;
  }
  else if ( count == 0 )  // graceful close
  {
    disconnect = true;
  }
  else
  {
    int errn = socket_errno;
    if ( errn != SOCKET_ERRNO( EWOULDBLOCK ) )
      disconnect = true;
  }
}


/* NOTE: If this changes, code in client.cpp must change - pause() and restart() use
   pre-encrypted values of 33 00 and 33 01.
   */
void Client::transmit_encrypted( const void* data, int len )
{
  THREAD_CHECKPOINT( active_client, 100 );
  const unsigned char* cdata = (const unsigned char*)data;
  unsigned char* pch;
  int i;
  int bidx;  // Offset in output byte
  EncryptedPktBuffer* outbuffer =
      PktHelper::RequestPacket<EncryptedPktBuffer>( ENCRYPTEDPKTBUFFER );
  pch = reinterpret_cast<unsigned char*>( outbuffer->getBuffer() );
  bidx = 0;
  THREAD_CHECKPOINT( active_client, 101 );
  for ( i = 0; i < len; i++ )
  {
    THREAD_CHECKPOINT( active_client, 102 );
    unsigned char ch = cdata[i];
    int nbits = Core::keydesc[ch].nbits;
    unsigned short inval = Core::keydesc[ch].bits_reversed;

    THREAD_CHECKPOINT( active_client, 103 );

    while ( nbits-- )
    {
      THREAD_CHECKPOINT( active_client, 104 );
      *pch <<= 1;
      if ( inval & 1 )
        *pch |= 1;
      bidx++;
      if ( bidx == 8 )
      {
        THREAD_CHECKPOINT( active_client, 105 );
        pch++;
        bidx = 0;
      }
      THREAD_CHECKPOINT( active_client, 106 );

      inval >>= 1;
    }
    THREAD_CHECKPOINT( active_client, 107 );
  }
  THREAD_CHECKPOINT( active_client, 108 );

  {
    int nbits = Core::keydesc[0x100].nbits;
    unsigned short inval = Core::keydesc[0x100].bits_reversed;

    THREAD_CHECKPOINT( active_client, 109 );

    while ( nbits-- )
    {
      THREAD_CHECKPOINT( active_client, 110 );
      *pch <<= 1;
      if ( inval & 1 )
        *pch |= 1;
      bidx++;
      THREAD_CHECKPOINT( active_client, 111 );
      if ( bidx == 8 )
      {
        pch++;
        bidx = 0;
      }
      THREAD_CHECKPOINT( active_client, 112 );

      inval >>= 1;
    }
  }
  THREAD_CHECKPOINT( active_client, 113 );

  if ( bidx == 0 )
  {
    pch--;
  }
  else
  {
    *pch <<= ( 8 - bidx );
  }
  THREAD_CHECKPOINT( active_client, 114 );

  passert_always( pch - reinterpret_cast<unsigned char*>( outbuffer->buffer ) + 1 <=
                  int( sizeof outbuffer->buffer ) );
  THREAD_CHECKPOINT( active_client, 115 );
  xmit( &outbuffer->buffer, static_cast<unsigned short>(
                                pch - reinterpret_cast<unsigned char*>( outbuffer->buffer ) + 1 ) );
  PktHelper::ReAddPacket( outbuffer );
  THREAD_CHECKPOINT( active_client, 116 );
}

void Client::transmit( const void* data, int len)
{
  ref_ptr<Core::BPacket> p;
  bool handled = false;
  // see if the outgoing packet has a SendFunction installed. If so call it. It may or may not
  // want us to continue sending the packet. If it does, handled will be false, and data, len, and p
  // will be altered. data has the new packet data to send, len the new length, and p, a ref counted
  // pointer to the packet object.
  //
  // If there is no outgoing packet script, handled will be false, and the passed params will be
  // unchanged.
  {
    PacketHookData* phd = nullptr;
    handled = GetAndCheckPacketHooked( this, data, phd );
    if ( handled )
    {
      Core::PolLock lock;
      CallOutgoingPacketExportedFunction( this, data, len, p, phd, handled );
    }
  }

  if ( handled )
    return;

  unsigned char msgtype = *(const char*)data;

  {
    Clib::SpinLockGuard guard( _fpLog_lock );
    if ( !fpLog.empty() )
    {
      fmt::Writer tmp;
      tmp << "Server -> Client: 0x" << fmt::hexu( msgtype ) << ", " << len << " bytes\n";
      Clib::fdump( tmp, data, len );
      FLEXLOG( fpLog ) << tmp.str() << "\n";
    }
  }

  std::lock_guard<std::mutex> guard( client_socket_mutex );
  if ( disconnect )
  {
    POLLOG_INFO << "Warning: Trying to send to a disconnected client! \n";
    fmt::Writer tmp;
    tmp << "Server -> Client: 0x" << fmt::hexu( msgtype ) << ", " << len << " bytes\n";
    Clib::fdump( tmp, data, len );
    POLLOG_INFO << tmp.str() << "\n";
    return;
  }

  if ( last_xmit_buffer )
  {
    Core::networkManager.queuedmode_iostats.sent[msgtype].count++;
    Core::networkManager.queuedmode_iostats.sent[msgtype].bytes += len;
  }
  Core::networkManager.iostats.sent[msgtype].count++;
  Core::networkManager.iostats.sent[msgtype].bytes += len;

  if ( encrypt_server_stream )
  {
    pause();
    transmit_encrypted( data, len );
  }
  else
  {
    xmit( data, static_cast<unsigned short>( len ) );
    // _xmit( client->csocket, data, len );
  }
}

void transmit( Client* client, const void* data, int len )
{
  Core::networkManager.clientTransmit->AddToQueue( client, data, len );
}

void Client::Disconnect()
{
  if ( this->isConnected() )
  {
    this->preDisconnect = true;
    Core::networkManager.clientTransmit->QueueDisconnection( this );
  }
}
}  // namespace Network
}  // namespace Pol
