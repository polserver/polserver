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
  bool has_incoming_data( unsigned int waitms, int* result = nullptr );
  bool accept( SOCKET* s, unsigned int mstimeout );
  bool accept( Socket* newsocket );
  bool recvbyte( unsigned char* byte, unsigned int waitms );
  bool recvdata_nowait( char* vdest, unsigned len, int* bytes_read );
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

class SocketLineReader
{
public:
  SocketLineReader( Socket& socket, unsigned int timeout_secs = 0, unsigned int max_linelength = 0,
                    bool disconnect_on_timeout = true )
      : _socket( socket ),
        _waitms( 500 ),
        _timeout_secs( timeout_secs ),
        _maxLinelength( max_linelength ),
        _disconnect_on_timeout( disconnect_on_timeout )
  {
  }
  bool try_readline( std::string& out, bool* timed_out = nullptr );
  bool readline( std::string& out, bool* timed_out = nullptr );

  void set_max_linelength( unsigned int max_linelength ) { _maxLinelength = max_linelength; }
  void set_wait( unsigned int waitms ) { _waitms = waitms; }
  void set_timeout( unsigned int timeout_secs ) { _timeout_secs = timeout_secs; }

  bool set_disconnect_on_timeout( bool disconnect ) { _disconnect_on_timeout = disconnect; }

private:
  Socket& _socket;
  std::string _currentLine;

  unsigned int _waitms;
  unsigned int _timeout_secs;
  unsigned int _maxLinelength;

  bool _disconnect_on_timeout;
};

}  // namespace Clib
}  // namespace Pol
#endif  // CLIB_WNSCKT_H
