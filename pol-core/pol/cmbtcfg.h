/*
History
=======
2005/23/11 MuadDib: Added warmode_wait timer for changing war mode.

Notes
=======

*/

struct CombatConfig {
    bool display_parry_success_messages;
    bool warmode_inhibits_regen;
	bool attack_self;
	unsigned long warmode_delay;
};

extern CombatConfig combat_config;
void read_combat_config();
