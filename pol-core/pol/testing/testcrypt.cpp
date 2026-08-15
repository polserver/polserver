/** @file
 *
 * @par History
 */

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "clib/logfacility.h"
#include "clib/network/sockets.h"
#include "clib/rawtypes.h"
#include "pol/crypt/blowfish.h"
#include "pol/crypt/crypt.h"
#include "pol/crypt/cryptengine.h"
#include "pol/crypt/cryptkey.h"
#include "pol/crypt/logincrypt.h"
#include "pol/crypt/md5.h"
#include "pol/crypt/twofish.h"
#include "pol/testing/testenv.h"

namespace Pol::Testing
{
using namespace Crypt;

namespace
{
// Keys of a 2.0.0 client, the version that still uses plain blowfish.
constexpr unsigned int key1_200 = 0x2D13A5FD;
constexpr unsigned int key2_200 = 0xA39D527F;

// The four seed bytes a client sends ahead of the login stream.
unsigned char client_seed[4] = { 0x7F, 0x00, 0x00, 0x01 };

std::vector<u8> sample_data( size_t len )
{
  std::vector<u8> data( len );
  for ( size_t i = 0; i < len; ++i )
    data[i] = static_cast<u8>( i * 7 + 3 );
  return data;
}

std::string hex( const u8* data, size_t len )
{
  std::string res;
  for ( size_t i = 0; i < len; ++i )
    res += fmt::format( "{:02x}", data[i] );
  return res;
}

// The 16 byte digest an MD5Crypt derived from Data, read back through its keystream.
std::string md5_digest( const std::string& data )
{
  MD5Crypt md5;
  md5.Init( (unsigned char*)data.data(), static_cast<unsigned int>( data.size() ) );
  u8 zeroes[16] = {};
  u8 out[16] = {};
  md5.Encrypt( zeroes, out, sizeof( out ) );
  return hex( out, sizeof( out ) );
}

// A connected loopback pair, so Receive() can be driven with the bytes a client would send.
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
    addr.sin_port = 0;
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
  SOCKET reader() const { return _reader; }
  bool write( const u8* data, int len )
  {
    return send( _writer, (const char*)data, len, 0 ) == len;
  }

private:
  SOCKET _writer = INVALID_SOCKET;
  SOCKET _reader = INVALID_SOCKET;
};

// Sends len bytes through the pair and returns what the engine made of them.
std::string received( CCryptBase& engine, const std::vector<u8>& data )
{
  SocketPair pair;
  if ( !pair.valid() || !pair.write( data.data(), static_cast<int>( data.size() ) ) )
    return "socket setup failed";
  std::vector<u8> buffer( data.size(), 0 );
  int count = engine.Receive( buffer.data(), static_cast<int>( buffer.size() ), pair.reader() );
  if ( count != static_cast<int>( data.size() ) )
    return fmt::format( "short read: {}", count );
  return hex( buffer.data(), buffer.size() );
}

void test_crypt_keys()
{
#define T_KEYS( version, type, k1, k2 )                                                  \
  UnitTest(                                                                              \
      []()                                                                               \
      {                                                                                  \
        TCryptInfo info{};                                                               \
        CalculateCryptKeys( version, info );                                             \
        return info.eType == ( type ) && info.uiKey1 == ( k1 ) && info.uiKey2 == ( k2 ); \
      },                                                                                 \
      true, "CalculateCryptKeys( \"" version "\" )" )

  // The three names that switch encryption off entirely.
  T_KEYS( "none", CRYPT_NOCRYPT, 0u, 0u );
  T_KEYS( "ignition", CRYPT_NOCRYPT, 0u, 0u );
  T_KEYS( "uorice", CRYPT_NOCRYPT, 0u, 0u );
  // Those three are matched by prefix, so a leading fragment picks them too.
  T_KEYS( "no", CRYPT_NOCRYPT, 0u, 0u );
  T_KEYS( "uo", CRYPT_NOCRYPT, 0u, 0u );
  // An empty version compares zero characters and so matches "none" as well.
  T_KEYS( "", CRYPT_NOCRYPT, 0u, 0u );

  // 2.0.0x is the one version spelled out rather than parsed.
  T_KEYS( "2.0.0x", CRYPT_BLOWFISH_TWOFISH, 0x2D13A5FDu, 0xA39D527Fu );

  // One version on each side of every boundary in the version ladder.
  T_KEYS( "1.25.35", CRYPT_OLD_BLOWFISH, 0x383477BCu, 0x02345CC6u );
  T_KEYS( "1.25.36", CRYPT_1_25_36, 0x387FC5CCu, 0x021510C6u );
  T_KEYS( "1.26.4", CRYPT_BLOWFISH, 0x32750719u, 0x0A2D100Bu );
  T_KEYS( "2.0.0", CRYPT_BLOWFISH, 0x2D13A5FDu, 0xA39D527Fu );
  T_KEYS( "2.0.3", CRYPT_BLOWFISH_TWOFISH, 0x2DBBB7CDu, 0xA3C95E7Fu );
  T_KEYS( "3.0.0", CRYPT_TWOFISH, 0x2D93A5FDu, 0xA3DD527Fu );
  T_KEYS( "5.0.0", CRYPT_TWOFISH, 0x2E93A5FDu, 0xA25D527Fu );
  T_KEYS( "7.0.0", CRYPT_TWOFISH, 0x2F93A5FDu, 0xA2DD527Fu );

  // A version with no dots parses as 0.0.0 and lands on the oldest engine.
  T_KEYS( "garbage", CRYPT_OLD_BLOWFISH, 0x2C13A5FDu, 0x0A31D527Fu );
#undef T_KEYS
}

void test_crypt_engine_factory()
{
#define T_ENGINE( type )                                                   \
  UnitTest(                                                                \
      []()                                                                 \
      {                                                                    \
        TCryptInfo info{ key1_200, key2_200, type };                       \
        std::unique_ptr<CCryptBase> engine{ create_crypt_engine( info ) }; \
        if ( engine == nullptr )                                           \
          return false;                                                    \
        engine->Init( client_seed );                                       \
        return true;                                                       \
      },                                                                   \
      true, "create_crypt_engine( " #type " )" )

  T_ENGINE( CRYPT_NOCRYPT );
  T_ENGINE( CRYPT_OLD_BLOWFISH );
  T_ENGINE( CRYPT_1_25_36 );
  T_ENGINE( CRYPT_BLOWFISH );
  T_ENGINE( CRYPT_BLOWFISH_TWOFISH );
  T_ENGINE( CRYPT_TWOFISH );
  // An unknown type falls back to no encryption rather than failing.
  T_ENGINE( static_cast<ECryptType>( 99 ) );
#undef T_ENGINE
}

void test_login_crypt()
{
  auto data = sample_data( 40 );
  auto plain = hex( data.data(), data.size() );

  // Each login variant is a keystream XOR, so a second pass with a freshly seeded
  // instance returns the original bytes.
#define T_LOGIN_ROUNDTRIP( method )                                   \
  UnitTest(                                                           \
      [&]()                                                           \
      {                                                               \
        std::vector<u8> once( data.size() ), twice( data.size() );    \
        LoginCrypt first, second;                                     \
        first.Init( client_seed, key1_200, key2_200 );                \
        second.Init( client_seed, key1_200, key2_200 );               \
        first.method( data.data(), once.data(), (int)data.size() );   \
        second.method( once.data(), twice.data(), (int)once.size() ); \
        if ( hex( once.data(), once.size() ) == plain )               \
          return std::string( "keystream left the data unchanged" );  \
        return hex( twice.data(), twice.size() );                     \
      },                                                              \
      plain, "LoginCrypt::" #method " round trip" )

  T_LOGIN_ROUNDTRIP( Decrypt );
  T_LOGIN_ROUNDTRIP( Decrypt_Old );
  T_LOGIN_ROUNDTRIP( Decrypt_1_25_36 );
#undef T_LOGIN_ROUNDTRIP

  // The three variants advance the key differently, so they must not agree.
  UnitTest(
      [&]()
      {
        std::vector<u8> a( data.size() ), b( data.size() ), c( data.size() );
        LoginCrypt current, old, v12536;
        current.Init( client_seed, key1_200, key2_200 );
        old.Init( client_seed, key1_200, key2_200 );
        v12536.Init( client_seed, key1_200, key2_200 );
        current.Decrypt( data.data(), a.data(), (int)data.size() );
        old.Decrypt_Old( data.data(), b.data(), (int)data.size() );
        v12536.Decrypt_1_25_36( data.data(), c.data(), (int)data.size() );
        return hex( a.data(), a.size() ) != hex( b.data(), b.size() ) &&
               hex( b.data(), b.size() ) != hex( c.data(), c.size() );
      },
      true, "LoginCrypt variants produce different keystreams" );

  // The seed alone decides the starting key.
  UnitTest(
      []()
      {
        LoginCrypt a, b;
        unsigned char other_seed[4] = { 0x0A, 0x00, 0x00, 0x02 };
        a.Init( client_seed, key1_200, key2_200 );
        b.Init( other_seed, key1_200, key2_200 );
        return a.lkey[0] != b.lkey[0] && a.lkey[1] != b.lkey[1];
      },
      true, "LoginCrypt::Init derives the key from the seed" );
}

void test_blowfish()
{
  auto data = sample_data( 64 );

  // Blowfish decryption feeds the ciphertext back into its state, so it is not its own
  // inverse; what it does guarantee is that the same seed yields the same stream.
  UnitTest(
      [&]()
      {
        std::vector<u8> a( data.size() ), b( data.size() );
        BlowFish first, second;
        first.Init();
        second.Init();
        first.Decrypt( data.data(), a.data(), (int)data.size() );
        second.Decrypt( data.data(), b.data(), (int)data.size() );
        return hex( a.data(), a.size() ) == hex( b.data(), b.size() ) &&
               hex( a.data(), a.size() ) != hex( data.data(), data.size() );
      },
      true, "BlowFish::Decrypt is deterministic and transforms the data" );

  // Past 21036 bytes the game table rotates mid-stream; decrypting that much in one
  // call must match decrypting it in two.
  UnitTest(
      []()
      {
        constexpr int len = CRYPT_GAMETABLE_TRIGGER + 2048;
        auto big = sample_data( len );
        std::vector<u8> one_call( len ), two_calls( len );
        BlowFish single, split;
        single.Init();
        split.Init();
        single.Decrypt( big.data(), one_call.data(), len );
        constexpr int first_half = CRYPT_GAMETABLE_TRIGGER - 100;
        split.Decrypt( big.data(), two_calls.data(), first_half );
        split.Decrypt( big.data() + first_half, two_calls.data() + first_half, len - first_half );
        return one_call == two_calls;
      },
      true, "BlowFish::Decrypt rotates the game table at 21036 bytes" );
}

void test_twofish()
{
  auto data = sample_data( 600 );
  auto plain = hex( data.data(), data.size() );

  // Twofish is a plain keystream XOR here, so two passes restore the input. 600 bytes
  // also forces the keystream block to be regenerated part way through.
  UnitTest(
      [&]()
      {
        std::vector<u8> once( data.size() ), twice( data.size() );
        TwoFish first, second;
        first.Init( client_seed );
        second.Init( client_seed );
        first.Decrypt( data.data(), once.data(), (int)data.size() );
        second.Decrypt( once.data(), twice.data(), (int)once.size() );
        if ( hex( once.data(), once.size() ) == plain )
          return std::string( "keystream left the data unchanged" );
        return hex( twice.data(), twice.size() );
      },
      plain, "TwoFish::Decrypt round trip across a keystream refill" );

  UnitTest(
      []()
      {
        TwoFish a, b;
        unsigned char other_seed[4] = { 0x0A, 0x00, 0x00, 0x02 };
        a.Init( client_seed );
        b.Init( other_seed );
        return hex( a.subData3, sizeof( a.subData3 ) ) != hex( b.subData3, sizeof( b.subData3 ) );
      },
      true, "TwoFish::Init keys the table from the seed" );
}

void test_md5()
{
  // Known digests, so a change in the MD5 implementation cannot pass unnoticed.
  UnitTest( []() { return md5_digest( "" ); }, std::string( "d41d8cd98f00b204e9800998ecf8427e" ),
            "MD5 of the empty input" );
  UnitTest( []() { return md5_digest( "abc" ); }, std::string( "900150983cd24fb0d6963f7d28e17f72" ),
            "MD5 of \"abc\"" );
  UnitTest( []() { return md5_digest( "The quick brown fox jumps over the lazy dog" ); },
            std::string( "9e107d9d372bb6826bd81d3542a419d6" ), "MD5 of a 43 byte input" );
  // 56 bytes is the length where the padding no longer fits and spills into a second
  // block; 200 bytes runs the compression function several times over.
  UnitTest(
      []()
      {
        auto data = sample_data( 56 );
        return md5_digest( std::string( data.begin(), data.end() ) );
      },
      std::string( "46c9907fc908ee68b1e7b8e71286a518" ), "MD5 of a 56 byte input" );
  UnitTest(
      []()
      {
        auto data = sample_data( 200 );
        return md5_digest( std::string( data.begin(), data.end() ) );
      },
      std::string( "4c79b81ac94bad7a875519ce6b964c66" ), "MD5 of a 200 byte input" );

  // The digest is used as a repeating 16 byte keystream.
  UnitTest(
      []()
      {
        auto data = sample_data( 48 );
        std::vector<u8> once( data.size() ), twice( data.size() );
        MD5Crypt first, second;
        first.Init( client_seed, sizeof( client_seed ) );
        second.Init( client_seed, sizeof( client_seed ) );
        first.Encrypt( data.data(), once.data(), (int)data.size() );
        second.Encrypt( once.data(), twice.data(), (int)once.size() );
        return hex( twice.data(), twice.size() ) == hex( data.data(), data.size() ) &&
               hex( once.data(), once.size() ) != hex( data.data(), data.size() );
      },
      true, "MD5Crypt::Encrypt round trip" );
}

void test_crypt_receive()
{
  auto data = sample_data( 32 );

  UnitTest(
      [&]()
      {
        CCryptNoCrypt engine;
        engine.Init( client_seed );
        return received( engine, data );
      },
      hex( data.data(), data.size() ), "CCryptNoCrypt::Receive passes the bytes through" );

  // For each engine the game branch has to match what its own game cipher produces.
#define T_RECEIVE_GAME( klass, expected_expr )                                      \
  UnitTest(                                                                         \
      [&]()                                                                         \
      {                                                                             \
        klass engine( key1_200, key2_200 );                                         \
        klass reference( key1_200, key2_200 );                                      \
        engine.Init( client_seed, CCryptBase::typeGame );                           \
        reference.Init( client_seed, CCryptBase::typeGame );                        \
        std::vector<u8> expected( data.size() );                                    \
        expected_expr;                                                              \
        return received( engine, data ) == hex( expected.data(), expected.size() ); \
      },                                                                            \
      true, #klass "::Receive decrypts the game stream" )

  T_RECEIVE_GAME( CCryptBlowfish,
                  reference.bfish.Decrypt( data.data(), expected.data(), (int)data.size() ) );
  T_RECEIVE_GAME( CCryptBlowfishOld,
                  reference.bfish.Decrypt( data.data(), expected.data(), (int)data.size() ) );
  T_RECEIVE_GAME( CCrypt12536,
                  reference.bfish.Decrypt( data.data(), expected.data(), (int)data.size() ) );
  T_RECEIVE_GAME( CCryptTwofish,
                  reference.tfish.Decrypt( data.data(), expected.data(), (int)data.size() ) );
  T_RECEIVE_GAME(
      CCryptBlowfishTwofish,
      ( reference.tfish.Decrypt( data.data(), expected.data(), (int)data.size() ),
        reference.bfish.Decrypt( expected.data(), expected.data(), (int)data.size() ) ) );
#undef T_RECEIVE_GAME

  // With no type given the engine picks one from the first byte: 0x80 once unmasked
  // means the client is still in the login stream.
#define T_RECEIVE_AUTO( klass, login_call )                                               \
  UnitTest(                                                                               \
      [&]()                                                                               \
      {                                                                                   \
        klass engine( key1_200, key2_200 );                                               \
        klass reference( key1_200, key2_200 );                                            \
        engine.Init( client_seed );                                                       \
        reference.Init( client_seed );                                                    \
        auto login_data = data;                                                           \
        login_data[0] = (u8)( CRYPT_AUTO_VALUE ^ (u8)engine.lcrypt.lkey[0] );             \
        std::vector<u8> expected( login_data.size() );                                    \
        login_call;                                                                       \
        return received( engine, login_data ) == hex( expected.data(), expected.size() ); \
      },                                                                                  \
      true, #klass "::Receive detects the login stream" )

  T_RECEIVE_AUTO( CCryptBlowfish, reference.lcrypt.Decrypt( login_data.data(), expected.data(),
                                                            (int)login_data.size() ) );
  T_RECEIVE_AUTO(
      CCryptBlowfishOld,
      reference.lcrypt.Decrypt_Old( login_data.data(), expected.data(), (int)login_data.size() ) );
  T_RECEIVE_AUTO( CCrypt12536, reference.lcrypt.Decrypt_1_25_36( login_data.data(), expected.data(),
                                                                 (int)login_data.size() ) );
  T_RECEIVE_AUTO( CCryptTwofish, reference.lcrypt.Decrypt( login_data.data(), expected.data(),
                                                           (int)login_data.size() ) );
  T_RECEIVE_AUTO(
      CCryptBlowfishTwofish,
      reference.lcrypt.Decrypt( login_data.data(), expected.data(), (int)login_data.size() ) );
#undef T_RECEIVE_AUTO

  // A first byte that does not unmask to 0x80 sends the same engine down the game path.
  UnitTest(
      [&]()
      {
        CCryptBlowfish engine( key1_200, key2_200 );
        CCryptBlowfish reference( key1_200, key2_200 );
        engine.Init( client_seed );
        reference.Init( client_seed );
        auto game_data = data;
        game_data[0] = (u8)( 0x21 ^ (u8)engine.lcrypt.lkey[0] );
        std::vector<u8> expected( game_data.size() );
        reference.bfish.Decrypt( game_data.data(), expected.data(), (int)game_data.size() );
        return received( engine, game_data ) == hex( expected.data(), expected.size() );
      },
      true, "CCryptBlowfish::Receive detects the game stream" );

  // The server side of the twofish engine encrypts what it sends back.
  UnitTest(
      [&]()
      {
        CCryptTwofish engine( key1_200, key2_200 );
        CCryptTwofish reference( key1_200, key2_200 );
        engine.Init( client_seed, CCryptBase::typeGame );
        reference.Init( client_seed, CCryptBase::typeGame );
        std::vector<u8> once( data.size() ), expected( data.size() );
        engine.Encrypt( data.data(), once.data(), (int)data.size() );
        reference.md5.Encrypt( data.data(), expected.data(), (int)data.size() );
        return hex( once.data(), once.size() ) == hex( expected.data(), expected.size() ) &&
               hex( once.data(), once.size() ) != hex( data.data(), data.size() );
      },
      true, "CCryptTwofish::Encrypt uses the md5 keystream" );

  // The base class encrypt is a no-op, which is what the unencrypted engines rely on.
  UnitTest(
      [&]()
      {
        CCryptNoCrypt engine;
        std::vector<u8> out( data.size(), 0xEE );
        engine.Encrypt( data.data(), out.data(), (int)data.size() );
        return hex( out.data(), out.size() ) == std::string( out.size() * 2, 'e' );
      },
      true, "CCryptBase::Encrypt leaves the buffer alone" );
}
}  // namespace

void crypt_test()
{
  test_crypt_keys();
  test_crypt_engine_factory();
  test_login_crypt();
  test_blowfish();
  test_twofish();
  test_md5();
  test_crypt_receive();
}
}  // namespace Pol::Testing
