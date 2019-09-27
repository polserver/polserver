#pragma once
#ifndef H_POLLINGWITHPOLL
#define H_POLLINGWITHPOLL

#include "../../passert.h"
#include "../sockets.h"

#ifdef _WIN32

// compatibility wrapper for windows
inline int poll( struct pollfd* fds, ULONG nfds, int timeout )
{
  return WSAPoll( fds, nfds, timeout );
};

#else

#include <poll.h>

#endif

namespace Pol
{
namespace Clib
{
class PollingWithPoll
{
public:
  explicit PollingWithPoll( SOCKET socket ) : default_timeout{0, 0}, processed( false )
  {
    fdList.fd = socket;
    reset();
  }

  void reset()
  {
    processed = false;

    fdList.events = 0;
    fdList.revents = 0;
  }

  void notify_on_incoming() { fdList.events |= POLLIN; }
  void notify_on_error() {}  // do nothing, poll() always notifies on error
  void notify_on_writable() { fdList.events |= POLLOUT; }

  bool incoming() { return ( processed ) ? ( ( fdList.revents & POLLIN ) != 0 ) : false; }
  bool error()
  {
    return ( processed ) ? ( ( fdList.revents & ( POLLHUP | POLLERR | POLLNVAL ) ) != 0 ) : false;
  }
  bool writable() { return ( processed ) ? ( ( fdList.revents & POLLOUT ) != 0 ) : false; }

  void set_timeout( int timeout_sec, int timeout_usec )
  {
    default_timeout.tv_sec = timeout_sec;
    default_timeout.tv_usec = timeout_usec;
  }

  int wait_for_events()
  {
    passert( valid_socket() );

    // ugly conversion, maybe change everything to use ms
    int timeout_ms = 1000 * default_timeout.tv_sec + default_timeout.tv_usec / 1000;

    // if timeout is non-zero but below 1ms, cap at 1ms
    if ( default_timeout.tv_usec != 0 && timeout_ms == 0 )
      timeout_ms = 1;

    int res = poll( &fdList, 1, timeout_ms );

    // only mark as processed if we don't have errors
    if ( res >= 0 )
      processed = true;

    return res;
  }

  bool valid_socket() { return fdList.fd != INVALID_SOCKET; }

private:
  pollfd fdList;  // a list of 1, this is a single poller

  timeval default_timeout;
  bool processed;
};
}  // namespace Clib
}  // namespace Pol
#endif