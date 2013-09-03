/*
History
=======
2005/03/09 Shinigami: Added Prop Delay [ms]

Notes
=======

*/


#ifndef WEPNTMPL_H
#define WEPNTMPL_H

#include <string>

#include "../action.h"
#include "../dice.h"
#include "../skillid.h"
#include "../equipdsc.h"
#include "itemdesc.h"

class Attribute;

class WeaponDesc : public EquipDesc
{
public:
	typedef EquipDesc base;
    WeaponDesc( u32 objtype, ConfigElem& elem, const Package* pkg );
    WeaponDesc(); // for dummy template
    virtual void PopulateStruct( BStruct* descriptor ) const;

    unsigned short get_random_damage() const;
    
    const Attribute* pAttr;
    //USKILLID skillid;
    unsigned short speed;
    unsigned short delay;

    ScriptDef hit_script;

    Dice damage_dice;

    bool projectile;
    unsigned short projectile_type;
    unsigned short projectile_anim;
    unsigned short projectile_sound;
    UACTION projectile_action;
    UACTION anim;
    UACTION mounted_anim;
    unsigned short hit_sound;
    unsigned short miss_sound;

    bool is_intrinsic;
	bool is_pc_weapon; // used to differentiate npc and pc intrinsics

    bool two_handed;

    unsigned short minrange;
    unsigned short maxrange;
};

#endif
