/*
History
=======
2005/03/05 Shinigami: schedule_spell - spell-delay removed

Notes
=======

*/

#include "charactr.h"

#include "../../clib/clib.h"
#include "../../clib/passert.h"

#include "../module/cfgmod.h"
#include "../module/osmod.h"

#include "../polclock.h"
#include "../scrsched.h"
#include "../spells.h"

#include "../module/uomod.h"
#include "../uoexec.h"


namespace Pol {
  namespace Mobile {
	bool Character::start_spell_script( Bscript::EScriptProgram* prog, Core::USpell* spell )
	{
	  return start_script( prog, true, new Bscript::BLong( spell->spell_id( ) ) );
	}

	void Character::schedule_spell( Core::USpell* spell )
	{
	  passert( spell_task == NULL );
	  new Core::SpellTask( &spell_task,
					 // polclock() + spell->delay() * POLCLOCKS_PER_SEC / 1000,
					 Core::polclock(),
					 this,
					 spell,
					 true );
	}

  }
}