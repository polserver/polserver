#ifndef CLIB_WNSCKT_H
#define CLIB_WNSCKT_H

#include <atomic>
#include <chrono>
#include <string>

#include "clib/Header_Windows.h"
#ifndef _WIN32
#include <sys/socket.h>

using SOCKET = int;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET ( SOCKET )( -1 )
#endif
#endif


namespace Pol::Clib
{
// How long a peer may accept nothing at all before its connection is considered dead.
// Two places wait out a stalled peer and both use this: Socket::send(), which blocks the
// calling thread, and the webserver's page scripts, which sleep and resume instead
// (pol/module/httpmod.cpp). They differ because a page script runs on the scripts thread
// while it holds the world lock, so it must never block; a thread-pool worker may.
//
// pol.cfg's StalledPeerTimeout sets it at startup and on config reload; clib cannot read
// the config itself, so pol hands the value down (pol/core.cpp apply_polcfg).
std::chrono::seconds stalled_peer_timeout();
void set_stalled_peer_timeout( std::chrono::seconds timeout );

// Time since a transfer last made any progress. Any progress at all restarts the budget,
// so a peer that keeps accepting data is never dropped for being slow -- only one that
// accepts nothing.
class StallBudget
{
public:
  void note_progress() { _deadline = clock::now() + stalled_peer_timeout(); }
  bool expired() const { return clock::now() >= _deadline; }

private:
  using clock = std::chrono::steady_clock;
  clock::time_point _deadline = clock::now() + stalled_peer_timeout();
};

class Socket
{
public:
  enum option
  {
    none = 0,
    nonblocking = 1,
    reuseaddr = 2
  };

  // Outcome of send_nowait(): everything reached the kernel, the peer's buffer is full
  // and the caller should retry with what is left, or the connection is gone.
  enum class SendResult
  {
    done,
    retry,
    error
  };

  Socket();
  explicit Socket( SOCKET sock );
  Socket( Socket&& sck );
  Socket& operator=( Socket&& sck );
  Socket( const Socket& ) = delete;
  Socket& operator=( const Socket& ) = delete;
  ~Socket();

  // Return false when the data did not (entirely) reach the kernel: the connection was
  // already gone, the peer stopped accepting data, or the send failed. Callers that
  // cannot do anything about it may ignore the result.
  bool write( const std::string& str );
  bool writeline( const std::string& s );

  // a zero connect_timeout means a blocking connect with the OS default timeout
  bool open( const char* ipaddr, unsigned short port,
             std::chrono::milliseconds connect_timeout = std::chrono::milliseconds::zero() );
  // loopback_only binds to 127.0.0.1 instead of all interfaces
  bool listen( unsigned short port, bool loopback_only = false );
  // a zero wait polls once and returns immediately
  bool has_incoming_data( std::chrono::milliseconds wait, int* result = nullptr );
  bool accept( Socket* newsocket );
  bool recvdata_nowait( char* vdest, unsigned len, int* bytes_read );
  bool recvdata( void* vdest, unsigned len, std::chrono::milliseconds wait );
  bool send( const void* data, unsigned length );
  SendResult send_nowait( const void* vdata, unsigned datalen, unsigned* nsent );
  bool connected() const;
  void close();

  bool is_local() const;

  std::string getpeername() const;
  struct sockaddr peer_address() const;

  SOCKET handle() const;
  SOCKET release_handle();

  void setsocket( SOCKET sck );
  void setpeer( struct sockaddr peer );

  // Records the options that listen()/accept() apply to the sockets they create.
  void set_options( option opt );
  // Switches an already-connected socket to non-blocking mode, for sockets this Socket
  // did not create itself (open() leaves its socket blocking). Returns false on failure,
  // leaving the socket blocking.
  bool set_nonblocking();

  void disable_nagle();

protected:
  void apply_socket_options( SOCKET sck );
  void apply_prebind_socket_options( SOCKET sck );

private:
  void HandleError();

  // atomic: close() may be called while another thread does I/O (aux connections
  // close from the reader thread while transmit tasks send); this keeps the
  // handle reads/writes race-free, but a concurrent sender can still lose the
  // race and act on an already-closed descriptor (specs/sockets/06).
  std::atomic<SOCKET> _sck;

  int _options;
  struct sockaddr _peer;
};

class SocketReader
{
public:
  // _wait is how long a single poll blocks; _timeout is how long the whole read may go
  // without progress before the peer is given up on. Different units, hence different types.
  SocketReader( Socket& socket, std::chrono::seconds timeout = std::chrono::seconds::zero(),
                bool disconnect_on_timeout = true )
      : _socket( socket ), _timeout( timeout ), _disconnect_on_timeout( disconnect_on_timeout )
  {
  }
  virtual ~SocketReader() = default;
  virtual bool try_read( std::string& out, bool* timed_out = nullptr ) = 0;
  bool read( std::string& out, bool* timed_out = nullptr );

  void set_wait( std::chrono::milliseconds wait ) { _wait = wait; }
  void set_timeout( std::chrono::seconds timeout ) { _timeout = timeout; }

  void set_disconnect_on_timeout( bool disconnect ) { _disconnect_on_timeout = disconnect; }

  bool connected() const { return _socket.connected(); }

protected:
  Socket& _socket;

  std::chrono::milliseconds _wait{ 500 };
  std::chrono::seconds _timeout;

  bool _disconnect_on_timeout;
  // set by try_read() when it consumed usable bytes; read() only refreshes its
  // timeout on progress
  bool _made_progress = false;
};

class SocketLineReader : public SocketReader
{
public:
  SocketLineReader( Socket& socket, std::chrono::seconds timeout = std::chrono::seconds::zero(),
                    unsigned int max_linelength = 0, bool disconnect_on_timeout = true )
      : SocketReader( socket, timeout, disconnect_on_timeout ), _maxLinelength( max_linelength )
  {
  }
  bool try_read( std::string& out, bool* timed_out = nullptr ) override;

  void set_max_linelength( unsigned int max_linelength ) { _maxLinelength = max_linelength; }

private:
  std::string _currentLine;
  // bytes at the front of _currentLine already handed out; the buffer is compacted
  // once this prefix outweighs the remainder rather than on every extracted line
  size_t _consumed = 0;
  unsigned int _maxLinelength;
};

class SocketByteReader : public SocketReader
{
public:
  SocketByteReader( Socket& socket, std::chrono::seconds timeout = std::chrono::seconds::zero(),
                    bool disconnect_on_timeout = true )
      : SocketReader( socket, timeout, disconnect_on_timeout )
  {
  }
  bool try_read( std::string& out, bool* timed_out = nullptr ) override;
};

}  // namespace Pol::Clib

#endif  // CLIB_WNSCKT_H
