#ifndef CLIB_WNSCKT_H
#define CLIB_WNSCKT_H

#include <string>

#include "../Header_Windows.h"
#ifndef WINDOWS
#include <sys/socket.h>

typedef int SOCKET;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET ( SOCKET )( -1 )
#endif
#endif

namespace Pol
{
namespace Clib
{
class Socket
{
public:
  enum option
  {
    none = 0,
    nonblocking = 1,
    reuseaddr = 2
  };

  Socket();
  explicit Socket( SOCKET sock );
  Socket( Socket&& sck );
  Socket& operator=( Socket&& sck );
  Socket( const Socket& ) = delete;
  Socket& operator=( const Socket& ) = delete;
  ~Socket();

  void write( const std::string& str );

  bool open( const char* ipaddr, unsigned short port );
  bool listen( unsigned short port );
  bool select( unsigned int seconds, unsigned int useconds );
  bool accept( SOCKET* s, unsigned int mstimeout );
  bool accept( Socket* newsocket );
  bool recvbyte( unsigned char* byte, unsigned int waitms );
  bool recvdata( void* vdest, unsigned len, unsigned int waitms );
  unsigned peek( void* vdest, unsigned len, unsigned int waitms );
  void send( const void* data, unsigned length );
  bool send_nowait( const void* vdata, unsigned datalen, unsigned* nsent );
  bool connected() const;
  void close();

  bool is_local() const;

  std::string getpeername() const;
  struct sockaddr peer_address() const;

  SOCKET handle() const;
  SOCKET release_handle();

  void setsocket( SOCKET sck );
  void setpeer( struct sockaddr peer );
  void takesocket( Socket& sck );

  void set_options( option opt );

  void disable_nagle();

protected:
  void apply_socket_options( SOCKET sck );
  void apply_prebind_socket_options( SOCKET sck );

private:
  void HandleError();

  SOCKET _sck;

  int _options;
  struct sockaddr _peer;
};
}  // namespace Clib
}  // namespace Pol
#endif  // CLIB_WNSCKT_H
