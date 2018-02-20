/** @file
 *
 * @par History
 */

#ifndef __TEXTCMD_H
#define __TEXTCMD_H

#include <cstddef>
#include <string>

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Network
{
class Client;
}

namespace Core
{
class wordicmp : public std::less<std::string>
{
public:
  bool operator()( const std::string& lhs, const std::string& rhs ) const;
};

bool process_command( Network::Client* client, const char* text, const u16* wtext = nullptr,
                      const char* lang = nullptr );
}
}
#endif
