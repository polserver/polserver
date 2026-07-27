#pragma once
#ifndef H_SINGLEPOLLER
#define H_SINGLEPOLLER

#include "clib/network/sockets.h"

#include "clib/network/singlepollers/pollingwithpoll.h"


namespace Pol::Clib
{
class SinglePoller
{
public:
  explicit SinglePoller( SOCKET socket ) : poller( socket ){};

  int wait_for_events() { return poller.wait_for_events(); }

  void set_timeout( std::chrono::milliseconds timeout ) { poller.set_timeout( timeout ); }

  bool incoming() { return poller.incoming(); }
  bool error() { return poller.error(); }
  bool writable() { return poller.writable(); }

  bool prepare( bool notify_writable )
  {
    if ( !poller.valid_socket() )
      return false;

    poller.reset();
    poller.notify_on_incoming();
    poller.notify_on_error();
    if ( notify_writable )
      poller.notify_on_writable();

    return true;
  }

private:
  PollingWithPoll poller;
};
}  // namespace Pol::Clib


#endif
