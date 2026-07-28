#pragma once
#ifndef H_MULTIPOLLER
#define H_MULTIPOLLER

#include <chrono>
#include <vector>

#include "clib/network/sockets.h"

// for the poll()/nfds_t compatibility wrappers
#include "clib/network/singlepollers/pollingwithpoll.h"


namespace Pol::Clib
{
// What the last wait() reported about one socket.
struct PollResult
{
  bool incoming = false;
  bool writable = false;
  bool error = false;
};

// Waits on several sockets at once, where SinglePoller waits on one.
//
// Use it when the sockets would otherwise be waited on one after another in a
// single thread: N separate waits cost N timeouts and only notice the last
// socket after inspecting all the others, while one wait over all of them
// returns as soon as any is ready. Sockets already being waited on by their
// own threads gain nothing from this - there each thread has to block on
// something anyway, and batching would only make every wakeup scan every
// socket.
//
// Rebuild it each pass: add() the sockets, wait() once, then read each back
// with the index add() returned.
class MultiPoller
{
public:
  void clear() { _fds.clear(); }
  bool empty() const { return _fds.empty(); }
  size_t size() const { return _fds.size(); }

  // Returns the index to pass to result(). Errors are always reported, so
  // there is no flag for them (poll() does not need to be asked).
  size_t add( SOCKET socket, bool notify_writable )
  {
    pollfd entry;
    entry.fd = socket;
    entry.events = static_cast<short>( POLLIN | ( notify_writable ? POLLOUT : 0 ) );
    entry.revents = 0;
    _fds.push_back( entry );
    return _fds.size() - 1;
  }

  // <0 on error, 0 if the timeout expired, otherwise the number of sockets
  // with something to report.
  int wait( std::chrono::milliseconds timeout )
  {
    _processed = false;
    if ( _fds.empty() )
      return 0;

    // The one place a duration becomes the plain integer milliseconds poll()
    // wants; see pollingwithpoll.h for why an int is always enough.
    int res = poll( _fds.data(), static_cast<nfds_t>( _fds.size() ),
                    static_cast<int>( timeout.count() ) );

    // only trust the revents if the wait itself succeeded
    if ( res >= 0 )
      _processed = true;

    return res;
  }

  PollResult result( size_t index ) const
  {
    if ( !_processed || index >= _fds.size() )
      return {};

    const short revents = _fds[index].revents;
    return { ( revents & POLLIN ) != 0, ( revents & POLLOUT ) != 0,
             ( revents & ( POLLHUP | POLLERR | POLLNVAL ) ) != 0 };
  }

private:
  std::vector<pollfd> _fds;
  bool _processed = false;
};
}  // namespace Pol::Clib

#endif
