/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/endian.h"

#include "polcfg.h"
#include "spells.h"
#include "tildecmd.h"


void invoke( Client* client, const char* spellidstr )
{
    u16 spellnum = static_cast<u16>(strtoul( spellidstr, NULL, 0 ));
    do_cast( client, spellnum );
}

void invoke( Client* client, const u16* wspellidstr )
{
	std::wstring wstr = L"";
	for(size_t i = 0; wspellidstr[i] != L'\0'; i++)
		wstr += static_cast<const wchar_t>(cfBEu16(wspellidstr[i]));
    u16 spellnum = static_cast<u16>(wcstoul( wstr.c_str(), NULL, 0 ));
    do_cast( client, spellnum );
}

bool process_tildecommand( Client* client, const char* textbuf )
{
    if (textbuf[0] == 'I' && textbuf[1] == 'N')
    {
		if ( config.loglevel >= 6 )
			cout << "INVOKE: " << textbuf << endl;

        invoke( client, textbuf+2 );
        return true;
    }
    return false;
}

bool process_tildecommand( Client* client, const u16* wtextbuf )
{
	using std::wcout;

    if (wtextbuf[0] == ctBEu16(L'I') && wtextbuf[1] == ctBEu16(L'N'))
    {
		if ( config.loglevel >= 6 )
		{
			wcout << L"INVOKE: ";
			for(size_t i=0; wtextbuf[i] != L'\0'; i++)
				wcout << static_cast<wchar_t>(cfBEu16(wtextbuf[i]));
			wcout << endl;
		}

        invoke( client, wtextbuf+2 );
        return true;
    }
    return false;
}
