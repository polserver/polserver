/** @file
 *
 * @par History
 */


#include "../plib/systemstate.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/msghandl.h"
#include "pktin.h"
#include "scrstore.h"

namespace Pol
{
namespace Core
{
void handle_help( Network::Client* client, PKTIN_9B* /*msg*/ )
{
  ref_ptr<Bscript::EScriptProgram> prog = find_script( "misc/help",
                                          true,
                                          Plib::systemstate.config.cache_interactive_scripts );
  if ( prog.get() != NULL )
  {
    client->chr->start_script( prog.get(), false );
  }
}
}
}