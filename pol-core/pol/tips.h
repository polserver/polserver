/** @file
 *
 * @par History
 */


#ifndef TIPS_H
#define TIPS_H

#include <string>

namespace Pol
{
namespace Network
{
class Client;
}
namespace Core
{
void load_tips();

// The tip window, 0xA6. tipnum names which of the tips directory's files the client is looking at,
// so it is zero for a line that came from a script instead.
void send_tip( Network::Client* client, const std::string& text, unsigned short tipnum = 0 );
}  // namespace Core
}  // namespace Pol
#endif
