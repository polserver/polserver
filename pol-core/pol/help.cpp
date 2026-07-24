/** @file
 *
 * @par History
 */

#include <stddef.h>

#include "bscript/eprog.h"
#include "clib/refptr.h"
#include "plib/systemstate.h"
#include "pol/mobile/charactr.h"
#include "pol/network/client.h"
#include "pol/network/pktin.h"
#include "pol/scrstore.h"


namespace Pol::Core
{
void handle_help( Network::Client* client, PKTIN_9B* /*msg*/ )
{
  ref_ptr<Bscript::EScriptProgram> prog =
      find_script( "misc/help", true, Plib::systemstate.config.cache_interactive_scripts );
  if ( prog.get() != nullptr )
  {
    client->chr->start_script( prog.get(), false );
  }
}
}  // namespace Pol::Core
