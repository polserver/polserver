/*
History
=======


Notes
=======

*/

#ifndef REPSYS_H
#define REPSYS_H

#include "polclock.h"
#include "npc.h"

namespace Pol {
  namespace Mobile {
	class Character;
  }
  namespace Network {
	class Client;
  }
  namespace Core {
	class NPC;
	class Party;

	class RepSystem
	{
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


	  friend void Mobile::Character::make_criminal( int level );
	  friend void Mobile::Character::make_murderer( bool newval );
	  friend void Mobile::Character::make_aggressor_to( Mobile::Character* chr );
	  friend void Mobile::Character::make_lawfullydamaged_to( Mobile::Character* chr );


	  friend void Mobile::Character::repsys_on_attack( Mobile::Character* defender );
	  friend void Mobile::Character::repsys_on_damage( Mobile::Character* defender );
	  friend void Mobile::Character::repsys_on_help( Mobile::Character* helper );

	  friend unsigned char Mobile::Character::hilite_color_idx( const Mobile::Character* seen_by ) const;
	  friend unsigned short Mobile::Character::name_color( const Mobile::Character* seen_by ) const;
	  friend unsigned char NPC::hilite_color_idx( const Mobile::Character* seen_by ) const;
	  friend unsigned short NPC::name_color( const Mobile::Character* seen_by ) const;

	  friend void show_repdata( Mobile::Character* looker, Mobile::Character* mob );
	};
  }
}
#endif
