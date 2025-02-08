/** @file
 *
 * @par History
 */


#include "wnsckt.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>

#include "esignal.h"
#include "logfacility.h"
#include "passert.h"
#include "strutil.h"

#include "singlepoller.h"

#if defined( WINDOWS )
#define SOCKET_ERRNO( x ) WSA##x
#define socket_errno WSAGetLastError()
typedef int socklen_t;

#else

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define SOCKET_ERRNO( x ) x
#define socket_errno errno

#endif

#ifndef SCK_WATCH
#define SCK_WATCH 0
#endif
namespace Pol
{
namespace Clib
{
Socket::Socket() : _sck( INVALID_SOCKET ), _options( none )
{
  memset( &_peer, 0, sizeof( _peer ) );
#ifdef _WIN32
  static bool init;
  if ( !init )
  {
    init = true;
    WSADATA dummy;
    WSAStartup( MAKEWORD( 1, 0 ), &dummy );
  }
#endif
}

Socket::Socket( SOCKET sock ) : _sck( sock ), _options( none )
{
  memset( &_peer, 0, sizeof( _peer ) );
}

Socket::Socket( Socket&& sock )
    : _sck( std::move( sock._sck ) ),
      _options( std::move( sock._options ) ),
      _peer( std::move( sock._peer ) )
{
  sock._sck = INVALID_SOCKET;
}

Socket& Socket::operator=( Socket&& sock )
{
  _sck = std::move( sock._sck );
  _options = std::move( sock._options );
  _peer = std::move( sock._peer );
  sock._sck = INVALID_SOCKET;
  return *this;
}

Socket::~Socket()
{
  close();
}

void Socket::setsocket( SOCKET sock )
{
  close();
  _sck = sock;
}

void Socket::set_options( option opt )
{
  _options = opt;
}

void Socket::setpeer( struct sockaddr peer )
{
  _peer = peer;
}

std::string Socket::getpeername() const
{
  struct sockaddr client_addr;  // inet_addr
  socklen_t addrlen = sizeof client_addr;
  if ( ::getpeername( _sck, &client_addr, &addrlen ) == 0 )
  {
    struct sockaddr_in* in_addr = (struct sockaddr_in*)&client_addr;
    if ( client_addr.sa_family == AF_INET )
      return inet_ntoa( in_addr->sin_addr );
    else
      return "(display error)";
  }
  else
    return "Error retrieving peer name";
}
struct sockaddr Socket::peer_address() const
{
  return _peer;
}

SOCKET Socket::handle() const
{
  return _sck;
}

SOCKET Socket::release_handle()
{
  SOCKET s = _sck;
  _sck = INVALID_SOCKET;
  return s;
}


bool Socket::open( const char* ipaddr, unsigned short port )
{
  close();
  _sck = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( _sck == INVALID_SOCKET
#ifndef WINDOWS
       || _sck < 0
#endif
  )
  {
    INFO_PRINTLN( "Unable to open socket in Socket::open()" );
    return false;
  }

  struct sockaddr_in sin;
  memset( &sin, 0, sizeof sin );
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr( ipaddr );
  sin.sin_port = htons( port );


  int res = connect( _sck, (struct sockaddr*)&sin, sizeof sin );

  if ( res == 0 )
  {
    return true;
  }
  else
  {
#ifdef _WIN32
    closesocket( _sck );
#else
    ::close( _sck );
#endif
    _sck = INVALID_SOCKET;
    return false;
  }
}

void Socket::disable_nagle()
{
  if ( _sck == INVALID_SOCKET )
    return;

  int tcp_nodelay = 1;
  int res = setsockopt( _sck, IPPROTO_TCP, TCP_NODELAY, (const char*)&tcp_nodelay,
                        sizeof( tcp_nodelay ) );
  if ( res < 0 )
  {
    throw std::runtime_error( "Unable to setsockopt (TCP_NODELAY) on socket, res=" +
                              Clib::tostring( res ) );
  }
}

void Socket::apply_socket_options( SOCKET sck )
{
  if ( _options & nonblocking )
  {
#ifdef _WIN32
    u_long nonblocking = 1;
    int res = ioctlsocket( sck, FIONBIO, &nonblocking );
#else
    int flags = fcntl( sck, F_GETFL );
    if ( flags == -1 )
      flags = O_NONBLOCK;
    else
      flags |= O_NONBLOCK;
    int res = fcntl( sck, F_SETFL, flags );
#endif
    if ( res < 0 )
    {
      throw std::runtime_error( "Unable to set listening socket to nonblocking mode, res=" +
                                tostring( res ) );
    }
  }
}

void Socket::apply_prebind_socket_options( SOCKET sck )
{
  if ( sck != INVALID_SOCKET && _options & reuseaddr )
  {
#ifndef WIN32
    int reuse_opt = 1;
    int res =
        setsockopt( sck, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_opt, sizeof( reuse_opt ) );
    if ( res < 0 )
    {
      throw std::runtime_error( "Unable to setsockopt (SO_REUSEADDR) on listening socket, res = " +
                                tostring( res ) );
    }
#endif
  }
}

bool Socket::listen( unsigned short port )
{
  struct sockaddr_in local;

  close();
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = INADDR_ANY;
  /* Port MUST be in Network Byte Order */
  local.sin_port = htons( port );
  memset( local.sin_zero, 0, sizeof( local.sin_zero ) );  // not needed, but for completeness

  _sck = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( _sck == INVALID_SOCKET )
  {
    close();
    return false;
  }

  apply_socket_options( _sck );
  apply_prebind_socket_options( _sck );

  if ( bind( _sck, (struct sockaddr*)&local, sizeof( local ) ) == -1 )
  {
    HandleError();
    return false;
  }
  if ( ::listen( _sck, SOMAXCONN ) == -1 )
  {
    HandleError();
    return false;
  }
  return true;
}

bool Socket::has_incoming_data( unsigned int waitms, int* result )
{
  if ( !connected() )
  {
    if ( result )
      *result = -1;
    return false;
  }
  SinglePoller poller( _sck );
  poller.set_timeout( waitms );

  if ( !poller.prepare( false ) )
  {
    if ( result )
      *result = -1;
    throw std::runtime_error( "Unable to poll socket=" + tostring( _sck ) );
  }

  int res = -1;
  do
  {
    res = poller.wait_for_events();
  } while ( res < 0 && !exit_signalled && socket_errno == SOCKET_ERRNO( EINTR ) );

  if ( result )
    *result = res;
  if ( poller.error() )
  {
    HandleError();
    close();
  }

  return poller.incoming();
}

bool Socket::accept( SOCKET* s, unsigned int /*mstimeout*/ )
{
  *s = ::accept( _sck, nullptr, nullptr );
  if ( *s != INVALID_SOCKET )
  {
    apply_socket_options( *s );
    return true;
  }
  else
  {
    *s = INVALID_SOCKET;
    return false;
  }
}

bool Socket::accept( Socket* newsocket )
{
  struct sockaddr client_addr;
  socklen_t addrlen = sizeof client_addr;
  SOCKET s = ::accept( _sck, &client_addr, &addrlen );
  if ( s != INVALID_SOCKET )
  {
    apply_socket_options( s );
    newsocket->setsocket( s );
    newsocket->setpeer( client_addr );
    return true;
  }
  return false;
}

bool Socket::connected() const
{
  return ( _sck != INVALID_SOCKET );
}

/* Read and clear the error value */
void Socket::HandleError()
{
#ifdef _WIN32
  int ErrVal;
#if SCK_WATCH
  static char ErrorBuffer[80];
#endif

  ErrVal = WSAGetLastError();
  WSASetLastError( 0 );

  switch ( ErrVal )
  {
  case WSAENOTSOCK:   /* Software caused connection to abort */
  case WSAECONNRESET: /* Arg list too long */
    close();
    break;

  default:
    close(); /*
                   gee, we'll close here,too.
                   if you want to _not_ close for _specific_ error codes,
                   feel more than free to make exceptions; but the general
                   rule should be, close on error.
                   */
    break;
  }

#if SCK_WATCH
  if ( FormatMessage( FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle( TEXT( "wsock32" ) ), ErrVal,
                      MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ErrorBuffer, sizeof ErrorBuffer,
                      nullptr ) == 0 )
  {
    sprintf( ErrorBuffer, "Unknown error code 0x%08x", ErrVal );
  }
  INFO_PRINTLN( std::string( ErrorBuffer ) );
#endif
#else
  close();
#endif
}

bool Socket::recvbyte( unsigned char* ch, unsigned int waitms )
{
  if ( !connected() )
    return false;

#if SCK_WATCH
  INFO_PRINTLN( "{L;1}" );
#endif

  int res;
  if ( !has_incoming_data( waitms, &res ) )
  {
    if ( res == -1 )
    {
      HandleError();
      close();
    }
    else if ( res == 0 )
    {
#if SCK_WATCH
      INFO_PRINTLN( "{TO}" );
#endif
    }

    return false;
  }


  res = recv( _sck, (char*)ch, 1, 0 );
  if ( res == 1 )
  {
#if SCK_WATCH
    INFO_PRINTLN( "{{{:#x}}}", *ch );
#endif
    return true;
  }
  else if ( res == 0 )
  {
#if SCK_WATCH
    INFO_PRINTLN( "{CLOSE}" );
#endif
    close();
    return false;
  }
  else
  {
    /* Can't time out here this is an ERROR! */
    HandleError();
    return false;
  }
}

bool Socket::recvdata_nowait( char* pdest, unsigned len, int* bytes_read )
{
  if ( bytes_read )
    *bytes_read = -1;

  if ( !connected() )
    return false;

#if SCK_WATCH
  INFO_PRINTLN( "{{L:{}}}", len );
#endif

  int res;
  res = ::recv( _sck, pdest, len, 0 );

  if ( bytes_read )
    *bytes_read = res;

  if ( res < 0 )
  {
    /* Can't time out here this is an ERROR! */
    HandleError();
    return false;
  }

  if ( res == 0 )
  {
#if SCK_WATCH
    INFO_PRINTLN( "{CLOSE}" );
#endif
    close();
    return false;
  }

  return true;
}

bool Socket::recvdata( void* vdest, unsigned len, unsigned int waitms )
{
  char* pdest = (char*)vdest;

  while ( len )
  {
    if ( !connected() )
      return false;

#if SCK_WATCH
    INFO_PRINTLN( "{{L:{}}}", len );
#endif
    int res;
    if ( !has_incoming_data( waitms, &res ) )
    {
      if ( res == -1 )
      {
        HandleError();
        close();
      }
      else if ( res == 0 )
      {
#if SCK_WATCH
        INFO_PRINTLN( "{TO}" );
#endif
      }

      return false;
    }


    res = ::recv( _sck, pdest, len, 0 );
    if ( res > 0 )
    {
#if SCK_WATCH
      char* tmp = pdest;
      INFO_PRINTLN( "{{R:{}[{}]}}", res, len );
#endif
      len -= res;
      pdest += res;

#if SCK_WATCH
      while ( res-- )
        INFO_PRINTLN( "{{{:#x}}}", (unsigned char)( *tmp++ ) );
#endif
    }
    else if ( res == 0 )
    {
#if SCK_WATCH
      INFO_PRINTLN( "{CLOSE}" );
#endif
      close();
      return false;
    }
    else
    {
      /* Can't time out here this is an ERROR! */
      HandleError();
      return false;
    }
  }
  return true;
}

unsigned Socket::peek( void* vdest, unsigned len, unsigned int wait_sec )
{
  ;
  char* pdest = (char*)vdest;

#if SCK_WATCH
  INFO_PRINTLN( "{{L:{}}}", len );
#endif

  int res;
  if ( !has_incoming_data( 1000 * wait_sec, &res ) )
  {
    if ( res == -1 )
    {
      HandleError();
      close();
    }
    else if ( res == 0 )
    {
#if SCK_WATCH
      INFO_PRINTLN( "{TO}" );
#endif
    }

    return 0;
  }


  res = ::recv( _sck, pdest, len, MSG_PEEK );
  if ( res > 0 )
  {
    return res;
  }
  else if ( res == 0 )
  {
#if SCK_WATCH
    INFO_PRINTLN( "{CLOSE}" );
#endif
    close();
    return 0;
  }
  else
  {
    /* Can't time out here this is an ERROR! */
    HandleError();
    return 0;
  }
}

void Socket::send( const void* vdata, unsigned datalen )
{
  const char* cdata = static_cast<const char*>( vdata );

  while ( datalen )
  {
    int res = ::send( _sck, cdata, datalen, 0 );
    if ( res < 0 )
    {
      int sckerr = socket_errno;
      if ( sckerr == SOCKET_ERRNO( EWOULDBLOCK ) )
      {
        // FIXME sleep
        continue;
      }
      else
      {
        INFO_PRINTLN( "Socket::send() error: {}", sckerr );
        HandleError();
        return;
      }
    }
    else
    {
      datalen -= res;
      cdata += res;
    }
  }
}

bool Socket::send_nowait( const void* vdata, unsigned datalen, unsigned* nsent )
{
  const char* cdata = static_cast<const char*>( vdata );

  *nsent = 0;

  while ( datalen )
  {
    int res = ::send( _sck, cdata, datalen, 0 );
    if ( res < 0 )
    {
      int sckerr = socket_errno;
      if ( sckerr == SOCKET_ERRNO( EWOULDBLOCK ) )
      {
        // FIXME sleep
        return false;
      }
      else
      {
        INFO_PRINTLN( "Socket::send_nowait() error: {}", sckerr );
        HandleError();
        return true;
      }
    }
    else
    {
      datalen -= res;
      cdata += res;
      *nsent += res;
    }
  }
  return true;
}

void Socket::write( const std::string& s )
{
  send( (void*)s.c_str(), static_cast<unsigned int>( s.length() ) );
}


void Socket::close()
{
  if ( _sck != INVALID_SOCKET )
  {
#ifdef _WIN32
    shutdown( _sck, 2 );  // 2 is both sides. defined in winsock2.h ...
    closesocket( _sck );
#else
    shutdown( _sck, SHUT_RDWR );
    ::close( _sck );
#endif
    _sck = INVALID_SOCKET;
  }
}

bool Socket::is_local() const
{
  std::string s = getpeername();
  return ( s == "127.0.0.1" );
}

bool is_invalid_readline_char( unsigned char c )
{
  return !isprint( c ) && c != '\n';
}

bool SocketLineReader::try_read( std::string& out, bool* timed_out )
{
  if ( timed_out )
    *timed_out = false;

  // check if there is already a line in the buffer
  auto pos_newline = _currentLine.find_first_of( "\n" );

  // If not, try to read more data
  if ( pos_newline == std::string::npos )
  {
    std::array<char, 4096> buffer;

    int res = -1;
    if ( !_socket.has_incoming_data( _waitms, &res ) )
    {
      if ( timed_out )
        *timed_out = true;
      INFO_PRINTLN( "::try_read no data" );
      return false;
    }
    int bytes_read = -1;
    if ( !_socket.recvdata_nowait( buffer.data(), static_cast<unsigned>( buffer.size() ),
                                   &bytes_read ) )
    {
      INFO_PRINTLN( "::try_read recv failed" );
      return false;
    }

    // store current line size so we don't need to search from the beginning again
    size_t oldSize = _currentLine.size();

    // remove invalid characters from buffer
    auto buffer_end =
        std::remove_if( buffer.begin(), buffer.begin() + bytes_read, is_invalid_readline_char );

    auto valid_char_count = std::distance( buffer.begin(), buffer_end );

    // nothing gained from these bytes
    if ( !valid_char_count )
    {
      INFO_PRINTLN( "::try_read no valid" );
      return false;
    }

    // append only the valid characters to the buffer
    _currentLine.append( buffer.data(), valid_char_count );

    // update position
    pos_newline = _currentLine.find_first_of( "\n", oldSize );
  }

  // note that std::string::npos is larger than any other number, so the conditon below will be
  // false only if there is a newline before the maximum line length or if the current line is still
  // small.
  if ( _maxLinelength > 0 && pos_newline > _maxLinelength && _currentLine.size() > _maxLinelength )
  {
    INFO_PRINTLN( "::try_read MAXLEN ({}) '{}'", pos_newline, _currentLine );
    out = _currentLine;
    _currentLine.clear();
    return false;
  }

  // Haven't found it yet
  if ( pos_newline == std::string::npos )
  {
    INFO_PRINTLN( "::try_read no newline" );
    return false;
  }
  auto end_newline = pos_newline + 1;
  INFO_PRINTLN( "::try_read socket '{}'", _currentLine );
  out = _currentLine.substr( 0, pos_newline );
  _currentLine.erase( 0, end_newline );

  return true;
}

bool SocketByteReader::try_read( std::string& out, bool* timed_out )
{
  if ( timed_out )
    *timed_out = false;

  std::array<char, 4096> buffer;

  int res = -1;
  if ( !_socket.has_incoming_data( _waitms, &res ) )
  {
    if ( timed_out )
      *timed_out = true;
    return false;
  }
  int bytes_read = -1;
  if ( !_socket.recvdata_nowait( buffer.data(), static_cast<unsigned>( buffer.size() ),
                                 &bytes_read ) )
    return false;

  out = std::string( buffer.data(), bytes_read );

  return true;
}

// Blocks until a whole line is received, waitms are over or maxlen is reached
bool SocketReader::read( std::string& out, bool* timed_out )
{
  out = "";
  if ( timed_out )
    *timed_out = false;

  const int max_timeouts = ( _timeout_secs * 1000 ) / _waitms;
  bool single_timed_out = false;

  int timeout_left = max_timeouts;
  while ( !Clib::exit_signalled && _socket.connected() )
  {
    if ( try_read( out, &single_timed_out ) )
    {
      INFO_PRINTLN( "::read '{}'", out );
      return true;
    }

    // if try_read() is false, string "out" should be empty unless the maxlen was reached.
    if ( !out.empty() )
    {
      INFO_PRINTLN( "::read failed non-empty '{}'", out );
      _socket.close();
      return false;
    }

    if ( _timeout_secs > 0 && single_timed_out )
    {
      timeout_left--;
      if ( timeout_left <= 0 )
      {
        if ( _disconnect_on_timeout || !timed_out )
          _socket.close();

        if ( timed_out )
          *timed_out = true;

        INFO_PRINTLN( "::read timeout" );
        return false;
      }
    }
    else
    {
      // refresh timeout counter
      timeout_left = max_timeouts;
    }
  }
  INFO_PRINTLN( "::read disconnect" );
  return false;
}


}  // namespace Clib
}  // namespace Pol
