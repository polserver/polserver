/** @file
 *
 * @par History
 */

#ifndef __TEXTCMD_H
#define __TEXTCMD_H

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "clib/rawtypes.h"

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

bool process_command( Network::Client* client, const std::string& text,
                      const std::string& lang = "" );

// The colour slot of an equip.cfg line: one value, a low-high range, or a comma separated list of
// either. Each entry becomes an inclusive [low,high] pair. Entries that do not parse are dropped
// and reported through *malformed, so one typo does not cost the rest of the line.
std::vector<std::pair<u16, u16>> parse_equip_colors( const std::string& spec, bool* malformed );
}  // namespace Core
}  // namespace Pol
#endif
