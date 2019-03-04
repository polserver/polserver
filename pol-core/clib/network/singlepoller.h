#pragma once

#include "sockets.h"
#include "pollingwithselect.h"

namespace Pol
{
namespace Clib
{
using DefaultPoller = PollingWithSelect;

template <class PollingStrategy = DefaultPoller>
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