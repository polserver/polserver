/** @file
 *
 * @par History
 */

#include <stddef.h>

#include "../bscript/eprog.h"
#include "../clib/refptr.h"
#include "../plib/systemstate.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/pktin.h"
#include "scrstore.h"


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
