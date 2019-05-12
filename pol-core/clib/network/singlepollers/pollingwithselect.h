#pragma once
#ifndef H_POLLINGWITHSELECT
#define H_POLLINGWITHSELECT

#include "../../passert.h"
#include "../sockets.h"

namespace Pol
{
namespace Clib
{
class PollingWithSelect
{
public:
  explicit PollingWithSelect( SOCKET socket )
      : socket( socket ), default_timeout{0, 0}, processed( false )
  {
    reset();
  }

  void reset()
  {
    processed = false;
    FD_ZERO( &c_recv_fd );
    FD_ZERO( &c_err_fd );
    FD_ZERO( &c_send_fd );
  }

  void notify_on_incoming() { FD_SET( socket, &c_recv_fd ); }
  void notify_on_error() { FD_SET( socket, &c_err_fd ); }
  void notify_on_writable() { FD_SET( socket, &c_send_fd ); }

  bool incoming() { return ( processed ) ? FD_ISSET( socket, &c_recv_fd ) : false; }
  bool error() { return ( processed ) ? FD_ISSET( socket, &c_err_fd ) : false; }
  bool writable() { return ( processed ) ? FD_ISSET( socket, &c_send_fd ) : false; }

  void set_timeout( int timeout_sec, int timeout_usec )
  {
    default_timeout.tv_sec = timeout_sec;
    default_timeout.tv_usec = timeout_usec;
  }

  int wait_for_events()
  {
    passert( valid_socket() );

    // Linux requires nfds to be the largest descriptor + 1, Windows doesn't care
    const int nfds = socket + 1;
    timeval c_select_timeout = default_timeout;

    int res = select( nfds, &c_recv_fd, &c_send_fd, &c_err_fd, &c_select_timeout );

    // only mark as processed if we don't have errors, otherwise the sets are the same as we
    // provided
    if ( res >= 0 )
      processed = true;

    return res;
  }

  bool valid_socket()
  {
#ifndef _WIN32
    // Linux and other non-windows OS can't handle FDs larger than FD_SETSIZE when using select()
    return socket != INVALID_SOCKET && socket < FD_SETSIZE;
#else
    return socket != INVALID_SOCKET;
#endif
  }

private:
  fd_set c_recv_fd;
  fd_set c_err_fd;
  fd_set c_send_fd;

  SOCKET socket;
  timeval default_timeout;
  bool processed;
};
}  // namespace Clib
}  // namespace Pol

#endif