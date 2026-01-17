/** @file
 *
 * @par History
 */


#include "tildecmd.h"

#include <iostream>
#include <stdlib.h>
#include <string>

#include "../clib/logfacility.h"
#include "../plib/systemstate.h"
#include "spells.h"


namespace Pol::Core
{
bool process_tildecommand( Network::Client* client, const std::string& textbuf )
{
  if ( textbuf.size() > 3 && textbuf[1] == 'I' && textbuf[2] == 'N' )
  {
    INFO_PRINTLN_TRACE( 6 )( "INVOKE: {}", textbuf );
    u16 spellnum = static_cast<u16>( strtoul( textbuf.c_str() + 3, nullptr, 0 ) );
    do_cast( client, spellnum );
    return true;
  }
  return false;
}
}  // namespace Pol::Core
