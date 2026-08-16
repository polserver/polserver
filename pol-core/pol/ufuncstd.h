/** @file
 *
 * @par History
 */

#ifndef __UFUNCSTD_H
#define __UFUNCSTD_H

#include <string>

#include "plib/uconst.h"

namespace Pol::Network
{
class Client;
}
namespace Pol::Mobile
{
class Character;
}

namespace Pol::Core
{
void send_sysmessage( Network::Client* client, const std::string& text,
                      unsigned short font = Plib::DEFAULT_TEXT_FONT,
                      unsigned short color = Plib::DEFAULT_TEXT_COLOR );
void send_nametext( Network::Client* client, const Mobile::Character* chr, const std::string& str );
}  // namespace Pol::Core

#endif
