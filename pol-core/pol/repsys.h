/** @file
 *
 * @par History
 */


#ifndef REPSYS_H
#define REPSYS_H

#include "polclock.h"

namespace Pol {
  namespace Mobile {
	class Character;
	class NPC;
  }
  namespace Network {
	class Client;
  }
  namespace Core {

	class RepSystem
	{
	  friend class Mobile::NPC;
	  friend class Mobile::Character;
	private:

	  static void on_pc_attacks_pc( Mobile::Character* attacker, Mobile::Character* defender );
	  static void on_pc_damages_pc( Mobile::Character* attacker, Mobile::Character* defender );
	  static void on_pc_helps_pc( Mobile::Character* helper, Mobile::Character* helped );

	  static void de_escalate( Mobile::Character* amy, Mobile::Character* bob );

	  static void restart_crim_timer( Mobile::Character* chr );
	  static unsigned char hilite_color_idx( const Mobile::Character* chr, const Mobile::Character* who );
	  static unsigned short name_color( const Mobile::Character* seen_by, const Mobile::Character* who );

	  static void repsys_task( Mobile::Character* chr );
	  static void schedule_repsys_task( Mobile::Character* chr, polclock_t runat );

	  static void show_repdata( Network::Client* client, Mobile::Character* mob );

	  friend void show_repdata( Mobile::Character* looker, Mobile::Character* mob );
	};
  }
}
#endif
