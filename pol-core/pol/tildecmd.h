/** @file
 *
 * @par History
 */

#ifndef __TILECMD_H
#define __TILECMD_H

#include "../clib/rawtypes.h"
#include "../clib/unicode.h"

namespace Pol
{
namespace Network
{
class Client;
}
namespace Core
{
bool process_tildecommand( Network::Client* client, const char* textbuf );
bool process_tildecommand( Network::Client* client, const Clib::UnicodeString& wtextbuf );
}
}
#endif
