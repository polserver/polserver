/*
History
=======


Notes
=======

*/

#ifndef SPELLS_H
#define SPELLS_H

#include <string>
#include <vector>

#include "../clib/refptr.h"

#include "action.h"
#include "scrdef.h"

class Character;
class Client;
class ConfigElem;
class Package;
class TargetCursor;
class USpell;

extern u16 spell_scroll_objtype_limits[7][2];

class USpellParams
{
public:
    USpellParams();
	USpellParams( ConfigElem& elem );
    // default ok USpellParams( const USpellParams& );

public:
	unsigned short manacost;
    unsigned short difficulty;
    unsigned short pointvalue;
    unsigned short delay;
	
     // unsigned short magery_req_;
	//unsigned short magery_auto_success_; 
};


class USpell
{
public:
	USpell( ConfigElem& elem, Package* pkg );

	const unsigned short spell_id() const;
    const std::string& name() const;

	void cast( Character* caster );
	unsigned short manacost() const;
    unsigned short delay() const;
    unsigned short difficulty() const;

	bool consume_reagents( Character* chr );
	bool check_mana( Character* chr );
	bool check_skill( Character* chr );
	void consume_mana( Character* chr ); // assumes check_mana has returned true
    void speak_power_words( Character* chr );
    UACTION animation() const;

    friend void register_spell( USpell *spell, unsigned short spellid );

protected:

    Package* pkg_;

	unsigned short spellid_;
    std::string name_;
    std::string power_words_;
    UACTION action_;

    typedef std::vector<unsigned short> RegList; 
    RegList reglist_;
    USpellParams params_;

	/* Only one of these will be set. */
    ScriptDef scriptdef_;
/*
    string script_;
	void (*f_)(Character* caster);
	TargetCursor *tcursor_;
*/
    
	static void register_spell( USpell *spell );
};
inline unsigned short USpell::manacost() const 
{ 
    return params_.manacost; 
}
inline unsigned short USpell::delay() const
{
    return params_.delay;
}
inline unsigned short USpell::difficulty() const
{
    return params_.difficulty;
}
inline const std::string& USpell::name() const
{
    return name_;
}
inline UACTION USpell::animation() const
{
    return action_;
}
inline const unsigned short USpell::spell_id() const
{
	return spellid_;
}

//extern USpell *spells[ SPELLID__COUNT+1 ];

extern std::vector<USpell*> spells2;
inline bool VALID_SPELL_ID( long spellid )
{
    return (spellid >= 1 && spellid <= long(spells2.size()));
}

void do_cast( Client *client, unsigned long spellid );
void clean_spells();


#include <time.h>
#include "schedule.h"
#include "reftypes.h"
class SpellTask : public OneShotTask
{
public:
    SpellTask( OneShotTask** handle, polclock_t run_when, Character* caster, USpell* spell, bool dummy );
    virtual void on_run();

private:
    CharacterRef caster_;
    USpell* spell_;
};


#endif
