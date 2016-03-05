/** @file
 *
 * @par History
 */


#include "charactr.h"

#include "../uoexec.h"



namespace Pol
{
namespace Mobile
{

bool Character::start_skill_script( Bscript::EScriptProgram* prog )
{
  return start_script( prog, true );
}

}
}