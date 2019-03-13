/** @file
 *
 * @par History
 */


#include "../../bscript/eprog.h"
#include "charactr.h"

namespace Pol
{
namespace Mobile
{
bool Character::start_skill_script( Bscript::Program* prog )
{
  return start_script( prog, true );
}
}
}
