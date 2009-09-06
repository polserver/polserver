/*
History
=======
2005/03/05 Shinigami: schedule_spell - spell-delay removed

Notes
=======

*/

#include "../../clib/stl_inc.h"
#include "../../clib/clib.h"
#include "../../clib/passert.h"

#include "../module/cfgmod.h"
#include "../module/osmod.h"
#include "../polclock.h"
#include "../scrsched.h"
#include "../spells.h"

#include "../uoemod.h"
#include "../uoexec.h"

#include "charactr.h"

bool Character::start_spell_script( EScriptProgram* prog, USpell* spell )
{
    return start_script( prog, true, new BLong(spell->spell_id()) );
}

void Character::schedule_spell( USpell* spell )
{
    passert( spell_task == NULL );
    new SpellTask( &spell_task, 
                  // polclock() + spell->delay() * POLCLOCKS_PER_SEC / 1000,
                   polclock(),
                   this, 
                   spell,
                   true );
}

