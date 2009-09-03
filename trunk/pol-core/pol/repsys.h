/*
History
=======


Notes
=======

*/

#ifndef REPSYS_H
#define REPSYS_H

#include "polclock.h"

#include "charactr.h"
#include "npc.h"
#include "party.h"

class Character;
class Client;
class NPC;
class Party;

class RepSystem
{
private:

    static void on_pc_attacks_pc( Character* attacker, Character* defender );
    static void on_pc_damages_pc( Character* attacker, Character* defender );
    static void on_pc_helps_pc( Character* helper, Character* helped );

    static void de_escalate( Character* amy, Character* bob );
    
    static void restart_crim_timer( Character* chr );
    static unsigned char hilite_color_idx( const Character* chr, const Character* who );
    static unsigned short name_color( const Character* seen_by, const Character* who );
    
    static void repsys_task( Character* chr );
    static void schedule_repsys_task( Character* chr, polclock_t runat );

    static void show_repdata( Client* client, Character* mob );


    friend void Character::make_criminal(long level);
    friend void Character::make_murderer( bool newval );
	friend void Character::make_aggressor_to( Character* chr);
	friend void Character::make_lawfullydamaged_to( Character* chr);


    friend void Character::repsys_on_attack( Character* defender );
    friend void Character::repsys_on_damage( Character* defender );
    friend void Character::repsys_on_help( Character* helper );

    friend unsigned char Character::hilite_color_idx( const Character* seen_by ) const;
    friend unsigned short Character::name_color( const Character* seen_by ) const;
    friend unsigned char NPC::hilite_color_idx( const Character* seen_by ) const;
    friend unsigned short NPC::name_color( const Character* seen_by ) const;

    friend void show_repdata( Character* looker, Character* mob );
};

#endif
