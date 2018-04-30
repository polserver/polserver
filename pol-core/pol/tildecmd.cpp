/** @file
 *
 * @par History
 */


#include "tildecmd.h"

#include <iostream>
#include <stdlib.h>
#include <string>

#include <format/format.h>
#include "../clib/unicode.h"
#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../plib/systemstate.h"
#include "spells.h"

namespace Pol
{
namespace Core
{
void invoke( Network::Client* client, const char* spellidstr )
{
  u16 spellnum = static_cast<u16>( strtoul( spellidstr, NULL, 0 ) );
  do_cast( client, spellnum );
}

void invoke( Network::Client* client, const std::string& wspellidstr )
{
  u16 spellnum = static_cast<u16>( strtoul( wspellidstr.c_str(), nullptr, 0 ) );
  do_cast( client, spellnum );
}

bool process_tildecommand( Network::Client* client, const char* textbuf )
{
  if ( textbuf[0] == 'I' && textbuf[1] == 'N' )
  {
    INFO_PRINT_TRACE( 6 ) << "INVOKE: " << textbuf << "\n";

    invoke( client, textbuf + 2 );
    return true;
  }
  return false;
}

bool process_tildecommand( Network::Client* client, const Clib::UnicodeString& wtextbuf )
{
  if ( wtextbuf.lengthc() >= 2 && wtextbuf[0] == 'I' && wtextbuf[1] == 'N' )
  {
    if ( Plib::systemstate.config.loglevel >= 6 )
    {
      fmt::Writer tmp;
      tmp << "INVOKE: ";
      tmp << wtextbuf.asAscii(true);
      tmp << "\n";
      INFO_PRINT << tmp.str();
    }

    invoke( client, wtextbuf.substr(2).asAscii(true) );
    return true;
  }
  return false;
}
}
}
