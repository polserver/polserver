/** @file
 *
 * @par History
 * - 2009/12/04 Turley:    Crypto cleanup - Tomi
 */


#include <algorithm>
#include <errno.h>
#include <iterator>
#include <mutex>
#include <stddef.h>
#include <string>
#include <vector>

#include "clib/fdump.h"
#include "clib/logfacility.h"
#include "clib/network/sockets.h"
#include "clib/passert.h"
#include "clib/refptr.h"
#include "clib/spinlock.h"
#include "pol/accounts/account.h"
#include "pol/crypt/cryptbase.h"
#include "pol/ctable.h"
#include "pol/globals/network.h"
#include "pol/globals/state.h"
#include "pol/network/client.h"
#include "pol/network/clientio.h"
#include "pol/network/clienttransmit.h"
#include "pol/network/packethelper.h"
#include "pol/network/packethooks.h"
#include "pol/network/packets.h"
#include "pol/packetscrobj.h"
#include "pol/polsem.h"
#include "pol/polsig.h"
#include <fmt/chrono.h>


namespace Pol::Network
{
PacketLog Client::start_log()
{
  std::string filename = "log/";
  filename += acct->name();
  filename += ".log";

  return session()->start_log( filename );
}

PacketLog Client::stop_log()
{
  return session()->stop_log();
}


PacketLog ThreadedClient::start_log( std::string filename )
{
  Clib::SpinLockGuard guard( _fpLog_lock );
  if ( !fpLog.empty() )
  {
    return PacketLog::Unchanged;  // already logging
  }

  fpLog = OPEN_FLEXLOG( filename, true );
  if ( !fpLog.empty() )
  {
    return PacketLog::Success;
  }

  return PacketLog::Error;
}

PacketLog ThreadedClient::stop_log()
{
  Clib::SpinLockGuard guard( _fpLog_lock );
  if ( !fpLog.empty() )
  {
    auto time_tm = Clib::localtime( time( nullptr ) );
    FLEXLOGLN( fpLog, "Log closed at {:%c}", time_tm );
    CLOSE_FLEXLOG( fpLog );
    fpLog.clear();
    return PacketLog::Success;
  }

  return PacketLog::Unchanged;
}


std::string ThreadedClient::ipaddrAsString() const
{
  return AddressToString( &this->ipaddr );
}

std::string ThreadedClient::ipaddrProxyAsString() const
{
  return AddressToString( &this->ipaddr_proxy );
}

void ThreadedClient::recv_remaining( int total_expected )
{
  int count;
  int max_expected = total_expected - bytes_received;

  if ( max_expected <= 0 )
    return;

  {
    std::lock_guard<std::mutex> lock( _socketMutex );
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
    int errn = Clib::socket_errno();
    if ( errn != Clib::sockerr::wouldblock )
      disconnect = true;
  }
}

void ThreadedClient::recv_remaining_nocrypt( int total_expected )
{
  int count;

  if ( total_expected - bytes_received <= 0 )
    return;

  {
    std::lock_guard<std::mutex> lock( _socketMutex );
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
    int errn = Clib::socket_errno();
    if ( errn != Clib::sockerr::wouldblock )
      disconnect = true;
  }
}

/* NOTE: If this changes, code in client.cpp must change - pause() and restart() use
   pre-encrypted values of 33 00 and 33 01.
   */
void huffman_compress( const unsigned char* data, size_t len, std::vector<unsigned char>& out )
{
  // Size the output from the codes first: at 11 bits per byte, a 65535-byte packet encodes to
  // more than 65535 bytes.
  size_t nbits = Core::keydesc[0x100].nbits;
  for ( size_t i = 0; i < len; ++i )
    nbits += Core::keydesc[data[i]].nbits;
  out.assign( ( nbits + 7 ) / 8, 0 );

  unsigned char* pch = out.data();
  int bidx = 0;  // bits already in *pch
  auto put = [&]( const Core::SVR_KEYDESC& key )
  {
    unsigned short inval = key.bits_reversed;
    for ( int n = key.nbits; n > 0; --n )
    {
      *pch = static_cast<unsigned char>( ( *pch << 1 ) | ( inval & 1 ) );
      inval >>= 1;
      if ( ++bidx == 8 )
      {
        ++pch;
        bidx = 0;
      }
    }
  };
  for ( size_t i = 0; i < len; ++i )
    put( Core::keydesc[data[i]] );
  put( Core::keydesc[0x100] );
  if ( bidx != 0 )
    *pch <<= ( 8 - bidx );
}

void ThreadedClient::transmit_encrypted( const void* data, int len )
{
  THREAD_CHECKPOINT( active_client, 100 );
  thread_local std::vector<unsigned char> compressed;
  huffman_compress( static_cast<const unsigned char*>( data ), len, compressed );
  THREAD_CHECKPOINT( active_client, 114 );

  // The compressed stream has no framing of its own, so output longer than xmit's 16-bit length
  // can go out in pieces. xmit encrypts each piece in place.
  unsigned char* pch = compressed.data();
  size_t left = compressed.size();
  while ( left > 0 )
  {
    const auto n = static_cast<unsigned short>( std::min<size_t>( left, 0xFFFF ) );
    xmit( pch, n );
    pch += n;
    left -= n;
  }
  THREAD_CHECKPOINT( active_client, 116 );
}

void Client::transmit( const void* data, int len )
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

  // Every packet carries a 16-bit length, so anything longer is a builder bug. Sending it
  // would put a truncated length on the wire.
  if ( len <= 0 || len > 0xFFFF )
  {
    POLLOG_ERRORLN( "Client#{}: refused to send a packet of {} bytes (type {:#x})", instance_, len,
                    len > 0 ? *static_cast<const unsigned char*>( data ) : 0 );
    return;
  }

  unsigned char msgtype = *(const char*)data;

  {
    Clib::SpinLockGuard guard( _fpLog_lock );
    if ( !fpLog.empty() )
    {
      std::string tmp = fmt::format( "Server -> Client: {:#x}, {} bytes\n", msgtype, len );
      Clib::fdump( std::back_inserter( tmp ), data, len );
      FLEXLOGLN( fpLog, tmp );
    }
  }

  std::lock_guard<std::mutex> guard( _socketMutex );
  if ( disconnect )
  {
    POLLOG_INFOLN( "Warning: Trying to send to a disconnected client! " );
    std::string tmp = fmt::format( "Server -> Client: {:#x}, {} bytes\n", msgtype, len );
    Clib::fdump( std::back_inserter( tmp ), data, len );
    POLLOG_INFOLN( tmp );
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
}  // namespace Pol::Network
