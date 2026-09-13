/** @file
 *
 * @par History
 */


#ifndef __SKILLS_H
#define __SKILLS_H
namespace Pol
{
namespace Network
{
class Client;
}
namespace Mobile
{
class Attribute;
class Character;
}  // namespace Mobile
namespace Core
{
bool CanUseSkill( Mobile::Character* chr );
bool StartSkillScript( Network::Client* client, const Mobile::Attribute* attrib );
}  // namespace Core
}  // namespace Pol
#endif
