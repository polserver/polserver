/** @file
 *
 * @par History
 */


#ifndef SOCKETSVC_H
#define SOCKETSVC_H

#include <chrono>

#include "clib/network/wnsckt.h"


namespace Pol::Clib
{
class SocketListener
{
public:
  explicit SocketListener( unsigned short port, bool loopback_only = false );
  SocketListener( unsigned short port, Socket::option opt, bool loopback_only = false );

  // Waits for a connection and takes it. Callers that have other sockets to
  // wait on at the same time should instead poll listen_socket() themselves
  // and call Accept() when it reports something.
  bool GetConnection( Socket* newsck, std::chrono::milliseconds timeout );

  SOCKET listen_socket() const { return _listen_sck.handle(); }

  // Takes an already-pending connection without waiting first. Only call this
  // after the listen socket reported incoming data; on a listener opened
  // without Socket::nonblocking it would otherwise block until one arrives.
  bool Accept( Socket* newsck ) { return _listen_sck.accept( newsck ); }

  friend class SocketClientThread;

private:
  Socket _listen_sck;
};

class SocketClientThread
{
public:
  explicit SocketClientThread( Socket&& S );
  virtual ~SocketClientThread() = default;
  void start();
  virtual void run() = 0;
  static void start_thread( SocketClientThread* instance );

protected:
  Socket _sck;
};
}  // namespace Pol::Clib

#endif
