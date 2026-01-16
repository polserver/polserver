/** @file
 *
 * @par History
 */


#include "../../bscript/eprog.h"
#include "charactr.h"


namespace Pol::Mobile
{
bool Character::start_skill_script( Bscript::EScriptProgram* prog )
{
  return start_script( prog, true );
}
}  // namespace Pol::Mobile
