/*
History
=======

Notes
=======

*/

#ifndef SCKUTIL_H
#define SCKUTIL_H

#include <string>
class Socket;

bool readline( Socket& sck, 
               std::string& s, 
               bool* timeout_exit = 0,
               unsigned long interchar_timeout_secs = 0,
               unsigned maxlen = 0
			  );

void writeline( Socket& sck, const std::string& s );

bool readstring( Socket& sck, string& s, unsigned long interchar_secs, unsigned length );

#endif
