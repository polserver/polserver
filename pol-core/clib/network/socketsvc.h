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
  bool GetConnection( Socket* newsck, std::chrono::milliseconds timeout );

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
