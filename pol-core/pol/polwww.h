/** @file
 *
 * @par History
 */


#ifndef POLWWW_H
#define POLWWW_H

#include <string>

namespace Pol::Core
{
void start_http_server();

// request/response helpers, exposed for the unit tests
std::string html_escape( const std::string& s );
bool constant_time_equal( const std::string& expected, const std::string& provided );
std::string http_decodestr( const std::string& s );
std::string decode_base64( const std::string& b64s );
bool legal_pagename( const std::string& page );
std::string get_pagetype( const std::string& page );
std::string reasonPhrase( int code );
}  // namespace Pol::Core

#endif
