/** @file
 *
 * @par History
 */

#ifndef __TILECMD_H
#define __TILECMD_H

#include <string>

namespace Pol
{
namespace Network
{
class Client;
}
namespace Core
{
bool process_tildecommand( Network::Client* client, const std::string& textbuf );
}  // namespace Core
}  // namespace Pol
#endif
