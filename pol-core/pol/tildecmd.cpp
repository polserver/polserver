/** @file
 *
 * @par History
 */


#include "tildecmd.h"

#include "polcfg.h"
#include "spells.h"

#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../plib/systemstate.h"

#include <iostream>

namespace Pol {
  namespace Core {

	void invoke( Network::Client* client, const char* spellidstr )
	{
	  u16 spellnum = static_cast<u16>( strtoul( spellidstr, NULL, 0 ) );
	  do_cast( client, spellnum );
	}

    void invoke( Network::Client* client, const u16* wspellidstr )
	{
	  std::wstring wstr = L"";
	  for ( size_t i = 0; wspellidstr[i] != L'\0'; i++ )
		wstr += static_cast<const wchar_t>( cfBEu16( wspellidstr[i] ) );
	  u16 spellnum = static_cast<u16>( wcstoul( wstr.c_str(), NULL, 0 ) );
	  do_cast( client, spellnum );
	}

    bool process_tildecommand( Network::Client* client, const char* textbuf )
	{
	  if ( textbuf[0] == 'I' && textbuf[1] == 'N' )
	  {
        INFO_PRINT_TRACE(6) << "INVOKE: " << textbuf << "\n";

		invoke( client, textbuf + 2 );
		return true;
	  }
	  return false;
	}

    bool process_tildecommand( Network::Client* client, const u16* wtextbuf )
	{
	  using std::wcout;

	  if ( wtextbuf[0] == ctBEu16( L'I' ) && wtextbuf[1] == ctBEu16( L'N' ) )
	  {
		if ( Plib::systemstate.config.loglevel >= 6 )
		{
          fmt::Writer tmp;
          tmp << "INVOKE: ";
		  for ( size_t i = 0; wtextbuf[i] != L'\0'; i++ )
            tmp << wcout.narrow( static_cast<wchar_t>( cfBEu16( wtextbuf[i] ) ), '?' );
          tmp << "\n";
          INFO_PRINT << tmp.str();
		}

		invoke( client, wtextbuf + 2 );
		return true;
	  }
	  return false;
	}
  }
}