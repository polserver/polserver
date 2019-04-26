#pragma once
#ifndef H_SINGLEPOLLER
#define H_SINGLEPOLLER

#include "sockets.h"

#include "singlepollers/pollingwithpoll.h"
//#include "singlepollers/pollingwithselect.h"

namespace Pol
{
namespace Clib
{
// This only works for Linux and Windows more recent than Windows Vista / Windows Server 2008. Let
// us know if anyone still needs Windows XP support (PollingWithSelect). If no one complains, we
// should remove pollingwithselect in the near future.
using PollingStrategy = PollingWithPoll;

class SinglePoller
{
public:
  explicit SinglePoller( SOCKET socket ) : poller( socket ){};

  int wait_for_events() { return poller.wait_for_events(); }

  // Note that not every poller will support usec resolution. I should find a way around it.
  void set_timeout( int timeout_sec, int timeout_usec )
  {
    poller.set_timeout( timeout_sec, timeout_usec );
  }

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
  PollingStrategy poller;
};
}  // namespace Clib
}  // namespace Pol

#endif