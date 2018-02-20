/** @file
 *
 * @par History
 * - 2005/09/23 Shinigami: added mf_SendStatus - to send full status packet to support extensions
 * - 2005/09/30 Shinigami: added Player Check to mf_SendStatus (crashed on NPCs)
 */

#include "../../bscript/berror.h"
#include "../mobile/charactr.h"
#include "../statmsg.h"
#include "../uoexhelp.h"
#include "uomod.h"


namespace Pol
{
namespace Module
{
using namespace Bscript;
BObjectImp* UOExecutorModule::mf_SendStatus( /* mob */ )
{
  Mobile::Character* chr;
  if ( getCharacterParam( exec, 0, chr ) )
  {
    if ( !chr->has_active_client() )
      return new BError( "No client attached" );

    if ( chr->logged_in() )
    {
      send_full_statmsg( chr->client, chr );
      return new BLong( 1 );
    }
    else
      return new BError( "Mobile must not be offline" );
  }
  else
    return new BError( "Invalid parameter type" );
}
}
}
