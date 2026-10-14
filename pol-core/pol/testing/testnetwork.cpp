/** @file
 *
 * @par History
 */

#include <chrono>
#include <cstring>
#include <map>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/asio/ip/network_v4.hpp>

#include "clib/network/sockets.h"
#include "clib/rawtypes.h"
#include "pol/crypt/cryptkey.h"
#include "pol/ctable.h"
#include "pol/globals/network.h"
#include "pol/network/client.h"
#include "pol/network/clientio.h"
#include "pol/network/clientthread.h"
#include "pol/network/cliface.h"
#include "pol/network/proxyprotocol.h"
#include "pol/testing/testenv.h"

namespace Pol::Testing
{
namespace
{
// Decodes a server-to-client Huffman stream up to its flush code. The codes come from the same
// table the encoder uses, read in the order the encoder emits their bits.
std::vector<u8> huffman_decode( const std::vector<unsigned char>& in, bool* flushed )
{
  std::map<std::pair<int, unsigned>, int> codes;
  for ( int sym = 0; sym <= 0x100; ++sym )
  {
    const auto& key = Core::keydesc[sym];
    unsigned emitted = 0;
    for ( int n = 0; n < key.nbits; ++n )
      emitted = ( emitted << 1 ) | ( ( key.bits_reversed >> n ) & 1 );
    codes[{ key.nbits, emitted }] = sym;
  }

  std::vector<u8> out;
  *flushed = false;
  unsigned code = 0;
  int nbits = 0;
  for ( unsigned char byte : in )
  {
    for ( int bit = 7; bit >= 0; --bit )
    {
      code = ( code << 1 ) | ( ( byte >> bit ) & 1 );
      ++nbits;
      auto itr = codes.find( { nbits, code } );
      if ( itr == codes.end() )
        continue;
      if ( itr->second == 0x100 )
      {
        *flushed = true;
        return out;
      }
      out.push_back( static_cast<u8>( itr->second ) );
      code = 0;
      nbits = 0;
    }
  }
  return out;
}

bool huffman_round_trips( const std::vector<u8>& data )
{
  std::vector<unsigned char> compressed;
  Network::huffman_compress( data.data(), data.size(), compressed );
  bool flushed = false;
  return huffman_decode( compressed, &flushed ) == data && flushed;
}

void test_huffman()
{
  UnitTest(
      []()
      {
        std::vector<u8> data;
        for ( int i = 0; i < 1024; ++i )
          data.push_back( static_cast<u8>( i ) );
        return huffman_round_trips( data );
      },
      true, "huffman_compress round-trips every byte value" );

  UnitTest(
      []()
      {
        std::vector<unsigned char> compressed( 10, 0xEE );
        Network::huffman_compress( nullptr, 0, compressed );
        return compressed.size();
      },
      size_t{ 1 }, "huffman_compress of nothing is the flush code alone" );

  // 65535 bytes of the longest code encode to 11/8 of that, more than a 16-bit length covers.
  u8 longest = 0;
  for ( int sym = 0; sym < 0x100; ++sym )
    if ( Core::keydesc[sym].nbits > Core::keydesc[longest].nbits )
      longest = static_cast<u8>( sym );
  const std::vector<u8> worst( 0xFFFF, longest );
  UnitTest(
      [&]()
      {
        std::vector<unsigned char> compressed;
        Network::huffman_compress( worst.data(), worst.size(), compressed );
        return compressed.size();
      },
      ( size_t{ 0xFFFF } * Core::keydesc[longest].nbits + Core::keydesc[0x100].nbits + 7 ) / 8,
      "huffman_compress sizes its output for 65535 bytes of 11-bit codes" );
  UnitTest( [&]() { return huffman_round_trips( worst ); }, true,
            "huffman_compress round-trips 65535 bytes of 11-bit codes" );
}

// A connected loopback pair. The accepted end is handed to a Client, which closes it.
class SocketPair
{
public:
  SocketPair()
  {
    SOCKET listener = socket( AF_INET, SOCK_STREAM, 0 );
    if ( listener == INVALID_SOCKET )
      return;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
    socklen_t addrlen = sizeof( addr );
    if ( bind( listener, (sockaddr*)&addr, addrlen ) == 0 && listen( listener, 1 ) == 0 &&
         getsockname( listener, (sockaddr*)&addr, &addrlen ) == 0 )
    {
      _writer = socket( AF_INET, SOCK_STREAM, 0 );
      if ( _writer != INVALID_SOCKET && connect( _writer, (sockaddr*)&addr, addrlen ) == 0 )
        _reader = accept( listener, nullptr, nullptr );
    }
    Clib::close_socket( listener );
  }
  ~SocketPair()
  {
    if ( _writer != INVALID_SOCKET )
      Clib::close_socket( _writer );
    if ( _reader != INVALID_SOCKET )
      Clib::close_socket( _reader );
  }
  SocketPair( const SocketPair& ) = delete;
  SocketPair& operator=( const SocketPair& ) = delete;

  bool valid() const { return _writer != INVALID_SOCKET && _reader != INVALID_SOCKET; }
  SOCKET release_reader() { return std::exchange( _reader, INVALID_SOCKET ); }
  bool write( const std::vector<u8>& data )
  {
    return send( _writer, (const char*)data.data(), static_cast<int>( data.size() ), 0 ) ==
           static_cast<int>( data.size() );
  }

private:
  SOCKET _writer = INVALID_SOCKET;
  SOCKET _reader = INVALID_SOCKET;
};

std::vector<u8> proxy_header( u8 command, u8 family_protocol, u16 payload_size )
{
  std::vector<u8> data = { 0x0D, 0x0A, 0x0D, 0x0A, 0x00, 0x0D, 0x0A, 0x51, 0x55, 0x49, 0x54, 0x0A };
  data.push_back( static_cast<u8>( 0x20 | command ) );
  data.push_back( family_protocol );
  data.push_back( static_cast<u8>( payload_size >> 8 ) );
  data.push_back( static_cast<u8>( payload_size & 0xFF ) );
  return data;
}

// Connects a Client from 127.0.0.1 with 127.0.0.1 allowed as a proxy, sends data and runs the
// receive state machine until it leaves the proxy states or disconnects. Returns the client's
// address and whether it was disconnected, or why the setup failed.
std::string after_proxy_bytes( const std::vector<u8>& data )
{
  SocketPair pair;
  if ( !pair.valid() || !pair.write( data ) )
    return "socket setup failed";

  sockaddr_in peer{};
  peer.sin_family = AF_INET;
  peer.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
  Crypt::TCryptInfo encryption{ 0, 0, Crypt::CRYPT_NOCRYPT };
  std::vector<boost::asio::ip::network_v4> proxies = {
      boost::asio::ip::make_network_v4( "127.0.0.1/32" ) };
  Network::Client client( *Core::networkManager.uo_client_interface, encryption,
                          reinterpret_cast<sockaddr&>( peer ), proxies );
  client.csocket = pair.release_reader();
  Clib::set_blocking( client.csocket, false );

  auto* session = client.session();
  std::string result;
  if ( session->recv_state != Network::ThreadedClient::RECV_STATE_PROXYPROTOCOLHEADER_WAIT )
    result = "proxy protocol not expected";
  for ( int tries = 0; result.empty() && tries < 200; ++tries )
  {
    Core::process_data( session );
    if ( session->disconnect )
      result = "disconnected";
    else if ( session->recv_state == Network::ThreadedClient::RECV_STATE_CRYPTSEED_WAIT )
      result = client.ipaddrAsString();
    else
      std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
  }
  client.Interface.deregister_client( &client );
  return result.empty() ? "still waiting" : result;
}

void test_proxy_protocol()
{
  // The payload is read into the receive buffer, so a header announcing more than the buffer
  // holds is refused before any of it is read.
  UnitTest(
      []()
      {
        auto data = proxy_header( PP_CMD_PROXY, ( PP_AF_INET << 4 ) | PP_TP_STREAM, 0xFFFF );
        data.resize( data.size() + 3000, 0xAB );
        return after_proxy_bytes( data );
      },
      std::string( "disconnected" ), "an oversized proxy header disconnects" );

  // Proxies may append type-length-value (TLV) records after the address block; those still fit
  // and are skipped.
  UnitTest(
      []()
      {
        const u16 tlvs = 200;
        auto data = proxy_header( PP_CMD_PROXY, ( PP_AF_INET << 4 ) | PP_TP_STREAM, 12 + tlvs );
        const std::vector<u8> addresses = { 10, 1, 2, 3, 127, 0, 0, 1, 0x12, 0x34, 0x13, 0x88 };
        data.insert( data.end(), addresses.begin(), addresses.end() );
        data.resize( data.size() + tlvs, 0x04 );
        return after_proxy_bytes( data );
      },
      std::string( "10.1.2.3" ), "a proxy header with TLVs sets the client address" );
}
}  // namespace

void network_test()
{
  test_huffman();
  test_proxy_protocol();
}
}  // namespace Pol::Testing
