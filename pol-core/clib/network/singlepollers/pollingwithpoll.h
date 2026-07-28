#pragma once
#ifndef H_POLLINGWITHPOLL
#define H_POLLINGWITHPOLL

#include <chrono>

#include "clib/passert.h"
#include "clib/network/sockets.h"

#ifdef _WIN32

// compatibility wrappers for windows
using nfds_t = ULONG;

inline int poll( struct pollfd* fds, nfds_t nfds, int timeout )
{
  return WSAPoll( fds, nfds, timeout );
};

#else

#include <poll.h>

#endif


namespace Pol::Clib
{
class PollingWithPoll
{
public:
  explicit PollingWithPoll( SOCKET socket ) : processed( false )
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

  void set_timeout( std::chrono::milliseconds timeout ) { timeout_ = timeout; }

  int wait_for_events()
  {
    passert( valid_socket() );

    // the one place a duration becomes the plain integer milliseconds poll() wants. A
    // timeout beyond ~24 days does not fit an int, and none of ours comes close: the
    // largest is os::OpenConnection's, which getParam already bounds to INT_MAX ms.
    int res = poll( &fdList, 1, static_cast<int>( timeout_.count() ) );

    // only mark as processed if we don't have errors
    if ( res >= 0 )
      processed = true;

    return res;
  }

  bool valid_socket() { return fdList.fd != INVALID_SOCKET; }

private:
  pollfd fdList;  // a list of 1, this is a single poller

  std::chrono::milliseconds timeout_{ 0 };
  bool processed;
};
}  // namespace Pol::Clib

#endif
