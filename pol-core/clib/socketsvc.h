/** @file
 *
 * @par History
 */


#ifndef SOCKETSVC_H
#define SOCKETSVC_H

#include "wnsckt.h"

namespace Pol
{
namespace Clib
{
class SocketListener
{
public:
  explicit SocketListener( unsigned short port );
  SocketListener( unsigned short port, Socket::option opt );
  bool GetConnection( unsigned int timeout_sec, unsigned int timeout_usec = 0 );

  void accept( Socket& newsck );

  friend class SocketClientThread;

private:
  Socket _listen_sck;
};

class SocketClientThread
{
public:
  explicit SocketClientThread( SocketListener& SL );
  explicit SocketClientThread( Socket& S );
  virtual ~SocketClientThread() {}
  void start();
  virtual void run() = 0;
  static void start_thread( SocketClientThread* instance );

protected:
  Socket _sck;
};
}  // namespace Clib
}  // namespace Pol
#endif
