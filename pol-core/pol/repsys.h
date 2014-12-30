/*
History
=======


Notes
=======

*/

#ifndef REPSYS_H
#define REPSYS_H

#include "polclock.h"

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
	class ExportedFunction;

	struct RepSys_Cfg
    {
      struct
      {
        unsigned short Murderer;
        unsigned short Criminal;
        unsigned short Attackable;
        unsigned short Innocent;
        unsigned short GuildAlly;
        unsigned short GuildEnemy;
        unsigned short Invulnerable;
      } NameColoring;

      struct
      {
        unsigned short CriminalFlagInterval;
        unsigned short AggressorFlagTimeout;
        bool PartyHelpFullCountsAsCriminal;
      } General;

      struct
      {
        ExportedFunction* NameColor;
        ExportedFunction* HighLightColor;
        ExportedFunction* OnAttack;
        ExportedFunction* OnDamage;
        ExportedFunction* OnHelp;
      } Hooks;

    };

	class RepSystem
	{
	  friend class NPC;
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
