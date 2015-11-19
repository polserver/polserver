/*
History
=======


Notes
=======

*/

#ifndef __SOCKETS_H
#define __SOCKETS_H

#ifdef _WIN32
#include "../clib/Header_Windows.h"
#define SOCKET_ERRNO(x) WSA##x
#define socket_errno WSAGetLastError()
typedef int socklen_t;

#else
// WTF! socket isn't a type.

	typedef int SOCKET;

#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <arpa/inet.h>
#	include <sys/socket.h>
#	include <sys/types.h>
#	include <netdb.h>
#	include <sys/time.h>
#	include <unistd.h>
#	include <fcntl.h>
#	include <errno.h>

#	define SOCKET_ERRNO(x) x
#	define socket_errno errno
#	define INVALID_SOCKET (SOCKET)(-1)

#endif

#endif
