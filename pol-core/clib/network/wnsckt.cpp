/** @file
 *
 * @par History
 */


#include "wnsckt.h"

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
  if (::getpeername( _sck, &client_addr, &addrlen ) == 0 )
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
    INFO_PRINT << "Unable to open socket in Socket::open()\n";
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
    flags |= O_NONBLOCK;
    int res = fcntl( sck, F_SETFL, O_NONBLOCK );
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
  if (::listen( _sck, SOMAXCONN ) == -1 )
  {
    HandleError();
    return false;
  }
  return true;
}

bool Socket::select( unsigned int seconds, unsigned int useconds, int* result )
{
  SinglePoller poller( _sck );
  poller.set_timeout( seconds, useconds );

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
  static char ErrorBuffer[80];

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
  INFO_PRINT << ErrorBuffer << "\n";
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
  INFO_PRINT << "{L;1}\n";
#endif

  int res;
  if ( !select( 0, waitms * 1000, &res ) )
  {
    if ( res == -1 )
    {
      HandleError();
      close();
    }
    else if ( res == 0 )
    {
#if SCK_WATCH
      INFO_PRINT << "{TO}\n";
#endif
    }

    return false;
  }


  res = recv( _sck, (char*)ch, 1, 0 );
  if ( res == 1 )
  {
#if SCK_WATCH
    INFO_PRINT.Format( "\{{:#X}\}\n" ) << ch;
#endif
    return true;
  }
  else if ( res == 0 )
  {
#if SCK_WATCH
    INFO_PRINT << "{CLOSE}\n";
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

bool Socket::recvdata( void* vdest, unsigned len, unsigned int waitms )
{
  char* pdest = (char*)vdest;

  while ( len )
  {
    if ( !connected() )
      return false;

#if SCK_WATCH
    INFO_PRINT << "{L:" << len << "}\n";
#endif
    int res;
    if ( !select( 0, waitms * 1000, &res ) )
    {
      if ( res == -1 )
      {
        HandleError();
        close();
      }
      else if ( res == 0 )
      {
#if SCK_WATCH
        INFO_PRINT << "{TO}\n";
#endif
      }

      return false;
    }


    res = ::recv( _sck, pdest, len, 0 );
    if ( res > 0 )
    {
#if SCK_WATCH
      char* tmp = pdest;
      INFO_PRINT.Format( "\{R:{}[{}]\}\n" ) << res << len;
#endif
      len -= res;
      pdest += res;

#if SCK_WATCH
      while ( res-- )
        INFO_PRINT.Format( "\{{:#X}\}\n" ) << (unsigned char)( *tmp++ );
#endif
    }
    else if ( res == 0 )
    {
#if SCK_WATCH
      INFO_PRINT << "{CLOSE}\n";
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
  INFO_PRINT << "{L:" << len << "}\n";
#endif

  int res;
  if ( !select( wait_sec, 0, &res ) )
  {
    if ( res == -1 )
    {
      HandleError();
      close();
    }
    else if ( res == 0 )
    {
#if SCK_WATCH
      INFO_PRINT << "{TO}\n";
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
    INFO_PRINT << "{CLOSE}\n";
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
        INFO_PRINT << "Socket::send() error: " << sckerr << "\n";
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
        INFO_PRINT << "Socket::send_nowait() error: " << sckerr << "\n";
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
}  // namespace Clib
}  // namespace Pol
