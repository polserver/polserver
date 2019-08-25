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

// TODO: move this function into the Socket class
// TODO: get rid of http_writeline, it's doing exactly the same as this function
void writeline( Socket& sck, const std::string& s );

}
}
#endif
