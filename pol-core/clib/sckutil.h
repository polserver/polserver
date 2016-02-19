/** @file
 *
 * @par History
 */


#ifndef SCKUTIL_H
#define SCKUTIL_H

#include <string>
namespace Pol
{
namespace Clib
{
class Socket;

bool readline( Socket& sck, std::string& s, bool* timeout_exit = 0,
               unsigned int interchar_timeout_secs = 0, unsigned maxlen = 0 );

void writeline( Socket& sck, const std::string& s );

bool readstring( Socket& sck, std::string& s, unsigned int interchar_secs, unsigned length );
}
}
#endif
