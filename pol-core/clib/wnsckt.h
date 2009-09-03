#ifndef __WNSCKT_H__
#define __WNSCKT_H__

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#endif


class Socket
{
public:
    enum option
    {
        none        = 0,
        nonblocking     = 1,
        reuseaddr       = 2
    };
    
    Socket();
    explicit Socket(int sock);
    Socket( Socket& sck ); // takes ownership
    virtual ~Socket();

    void write( const string& str );

    bool open( const char* ipaddr, unsigned short port );
    bool listen( unsigned short port );
    bool select( unsigned long seconds, unsigned long useconds );
	bool accept(int *s, unsigned long mstimeout);
    bool accept( Socket& newsocket );
    bool recvbyte( unsigned char* byte, unsigned long waitms );
    bool recvdata( void *vdest, unsigned len, unsigned long waitms );
    unsigned peek( void *vdest, unsigned len, unsigned long waitms );
    void send( const void* data, unsigned length );
    bool send_nowait( const void* vdata, unsigned datalen, unsigned* nsent );
    bool connected() const;
    void close();

    bool is_local();

    std::string getpeername() const;
    struct sockaddr peer_address() const;
    
    int handle() const;
    int release_handle();

    void setsocket( int sck );
    void setpeer( struct sockaddr peer );
    void takesocket( Socket& sck );

    void set_options( option opt );
protected:
    void apply_socket_options( int sck );
    void apply_prebind_socket_options( int sck );

private:
	void HandleError();
    
    int _sck;
    int _options;
    struct sockaddr _peer;
};

#endif
