/*
History
=======


Notes
=======

*/

#ifndef __SKILLS_H
#define __SKILLS_H
namespace Pol {
  namespace Core {
	bool CanUseSkill( Network::Client* client );
	bool StartSkillScript( Network::Client *client, const Mobile::Attribute* attrib );
  }
}
#endif
