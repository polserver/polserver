/** @file
 *
 * @par History
 * - 2005/03/05 Shinigami: schedule_spell - spell-delay removed
 */


#include <stddef.h>

#include "../../clib/passert.h"
#include "../polclock.h"
#include "../spells.h"
#include "charactr.h"


namespace Pol::Mobile
{
bool Character::start_spell_script( Bscript::EScriptProgram* prog, Core::USpell* spell )
{
  return start_script( prog, true, new Bscript::BLong( spell->spell_id() ) );
}

void Character::schedule_spell( Core::USpell* spell )
{
  passert( spell_task == nullptr );
  new Core::SpellTask( &spell_task,
                       // polclock() + spell->delay() * POLCLOCKS_PER_SEC / 1000,
                       Core::polclock(), this, spell, true );
}
}  // namespace Pol::Mobile
