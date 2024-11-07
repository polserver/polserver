/** @file
 *
 * @par History
 * - 2005/03/01 Shinigami: added MAX_NUMER_REALMS for use in MSGBF_SUB18_ENABLE_MAP_DIFFS
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: shadowrealm_count definition fixed
 * - 2009/12/02 Turley:    added TerMur - Tomi
 */


#ifndef REALMS_H
#define REALMS_H

#include <map>
#include <string>
#include <vector>

namespace Pol
{
namespace Realms
{
class Realm;
}
namespace Core
{
Realms::Realm* find_realm( const std::string& name );
void add_realm( const std::string& name, Realms::Realm* base, bool has_decay = true );
bool defined_realm( const std::string& name );
void remove_realm( const std::string& name );

// Support up to 5 Maps: Britannia, Britannia_alt, Ilshenar, Malas, Tokuno, TerMur
#define MAX_NUMER_REALMS 6
}
}
#endif
