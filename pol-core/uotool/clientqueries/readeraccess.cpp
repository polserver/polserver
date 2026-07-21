/** @file
 * uotool's process-wide reader instance + its load-on-first-use guards.
 *
 * @par History
 */

#include "readeraccess.h"

#include "../../plib/clientfiles/uoclientfiles.h"

namespace Pol::Plib
{
UoClientFiles& uofiles()
{
  static UoClientFiles instance;
  return instance;
}
}  // namespace Pol::Plib

namespace Pol::UoTool
{
void ensure_map( Pol::Plib::UoClientFiles& uof )
{
  if ( !uof.rawmap_loaded() )
    uof.rawmapfullread();
}
void ensure_statics( Pol::Plib::UoClientFiles& uof )
{
  if ( !uof.rawstatics_loaded() )
    uof.rawstaticfullread();
}
}  // namespace Pol::UoTool
