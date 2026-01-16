/** @file
 *
 * @par History
 */

#ifndef __UFUNCSTD_H
#define __UFUNCSTD_H

namespace Pol::Core
{
void send_sysmessage( Network::Client* client, const std::string& text, unsigned short font = 3,
                      unsigned short color = 0x3B2 );
void send_nametext( Network::Client* client, const Mobile::Character* chr, const std::string& str );
}  // namespace Pol::Core

#endif
