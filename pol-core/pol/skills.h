/** @file
 *
 * @par History
 */


#ifndef __SKILLS_H
#define __SKILLS_H
namespace Pol {
    namespace Network {
        class Client;
    }
    namespace Mobile {
        class Attribute;
    }
  namespace Core {
	bool CanUseSkill( Network::Client* client );
	bool StartSkillScript( Network::Client *client, const Mobile::Attribute* attrib );
  }
}
#endif
