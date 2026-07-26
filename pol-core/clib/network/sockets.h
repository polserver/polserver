/** @file
 *
 * @par History
 */


#ifndef __SOCKETS_H
#define __SOCKETS_H

#ifdef _WIN32
#include "clib/Header_Windows.h"
typedef int socklen_t;

#else
// WTF! socket isn't a type.

using SOCKET = int;

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define INVALID_SOCKET ( SOCKET )( -1 )

#endif


// Verbose per-operation socket tracing, off unless the build defines SCK_WATCH=1.
#ifndef SCK_WATCH
#define SCK_WATCH 0
#endif


namespace Pol::Clib
{
// Guard tracing with `if constexpr ( sck_watch )` rather than #if, so the debug code is
// always compiled (and cannot rot) while the optimizer drops it when disabled.
inline constexpr bool sck_watch = SCK_WATCH != 0;

// The error left by the last socket operation on this thread.
inline int socket_errno()
{
#ifdef _WIN32
  return WSAGetLastError();
#else
  return errno;
#endif
}

// Resets the error left by the last socket operation, so a later read cannot pick up
// a stale value.
inline void clear_socket_errno()
{
#ifdef _WIN32
  WSASetLastError( 0 );
#else
  errno = 0;
#endif
}

// Socket error codes named for the condition they describe, so call sites do not
// have to know which platform spells it which way.
namespace sockerr
{
#ifdef _WIN32
inline constexpr int wouldblock = WSAEWOULDBLOCK;
inline constexpr int intr = WSAEINTR;
// Winsock reports an in-progress non-blocking connect as WSAEWOULDBLOCK. WSAEINPROGRESS
// is a different condition there (a blocking call is already running), so it must not be
// "corrected" to that.
inline constexpr int inprogress = WSAEWOULDBLOCK;
#else
inline constexpr int wouldblock = EWOULDBLOCK;
inline constexpr int intr = EINTR;
inline constexpr int inprogress = EINPROGRESS;
#endif
}  // namespace sockerr

// Releases the descriptor. Callers that want the peer notified first call
// shutdown_both() before this.
inline void close_socket( SOCKET sck )
{
#ifdef _WIN32
  closesocket( sck );
#else
  ::close( sck );
#endif
}

// Shuts down both directions, so a blocked peer wakes up and the FIN goes out.
inline void shutdown_both( SOCKET sck )
{
#ifdef _WIN32
  shutdown( sck, SD_BOTH );
#else
  shutdown( sck, SHUT_RDWR );
#endif
}

// Returns false if the mode could not be changed.
inline bool set_blocking( SOCKET sck, bool blocking )
{
#ifdef _WIN32
  u_long nonblocking = blocking ? 0 : 1;
  return ioctlsocket( sck, FIONBIO, &nonblocking ) == 0;
#else
  int flags = fcntl( sck, F_GETFL );
  if ( flags == -1 )
    return false;
  flags = blocking ? ( flags & ~O_NONBLOCK ) : ( flags | O_NONBLOCK );
  return fcntl( sck, F_SETFL, flags ) == 0;
#endif
}
}  // namespace Pol::Clib

#endif
