/*
History
=======
2005/03/09 Shinigami: Added Prop Delay [ms]

Notes
=======

*/

#ifndef WEAPON_H
#define WEAPON_H

#include <iosfwd>
#include <memory>
#include <string>

#include "../action.h"

#include "equipmnt.h"

#include "../scrdef.h"
#include "../skillid.h"

class Attribute;
class Character;
class WeaponDesc;
class UWeapon;


class UWeapon : public Equipment
{
    typedef Equipment base;
public:

    unsigned short speed() const;
    unsigned short delay() const;
	unsigned short damage_mod() const;
    const Attribute& attribute() const;
    unsigned short get_random_damage() const;
    bool is_projectile() const;
    bool is_intrinsic() const;
    bool consume_projectile( UContainer* cont ) const;
    bool in_range( const Character* wielder,
                   const Character* target ) const;
    unsigned short projectile_sound() const;
    unsigned short projectile_anim() const;
    // UACTION projectile_action() const;
    UACTION anim() const;
    UACTION mounted_anim() const;
    unsigned short hit_sound() const;
    unsigned short miss_sound() const;

    const WeaponDesc& descriptor() const;
    const ScriptDef& hit_script() const;
    void set_hit_script( const std::string& scriptname );

    virtual Item* clone() const;

protected:
    virtual void printProperties( std::ostream& os ) const;
    virtual void readProperties( ConfigElem& elem );
    //BObjectImp* script_member( const char *membername );
    virtual BObjectImp* get_script_member( const char *membername ) const;
    virtual BObjectImp* get_script_member_id( const int id ) const; ///id test
    virtual BObjectImp* set_script_member( const char *membername, const std::string& value );
    virtual BObjectImp* set_script_member( const char *membername, int value );
    virtual BObjectImp* set_script_member_double( const char *membername, double value );
    virtual BObjectImp* set_script_member_id( const int id , const std::string& value ); //id test
    virtual BObjectImp* set_script_member_id( const int id , int value ); //id test
    virtual BObjectImp* set_script_member_id_double( const int id , double value ); //id test
    virtual bool script_isa( unsigned isatype ) const;
    UWeapon( const WeaponDesc& descriptor, const WeaponDesc* permanent_descriptor );
	~UWeapon();
    friend class Item;
    friend void load_data();
    friend UWeapon* create_intrinsic_weapon( const char* name, ConfigElem& elem, const Package* pkg );
    friend void load_weapon_templates();
    friend void load_intrinsic_weapons();

private:

    /* 
		max repair % in template
		quality(0-200%), repair( 0-125%)
		effective repair is min(repair,100%)
		quality drops after each repair attempt
		effects on damage/speed: damage * f( quality, effective repair)
			f ranges from .5 to 1.5
		effects on speed: speed * g(quality, effective_repair)
			g ranges from .75 to 1.25
    */


    const WeaponDesc* tmpl;

    ScriptDef hit_script_;
};

inline const ScriptDef& UWeapon::hit_script() const 
{ 
    return hit_script_; 
}


void load_weapon_templates();
void unload_weapon_templates();
bool isa_weapon( u32 objtype );

UWeapon* find_intrinsic_weapon( const std::string& name );
UWeapon* create_intrinsic_weapon( ConfigElem& elem );
UWeapon* create_intrinsic_weapon_from_npctemplate( ConfigElem& elem, const Package* pkg );

#endif
