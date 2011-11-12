/*
History
=======
2011/11/10 Tomi:	Added file

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfile.h"
#include "../clib/rawtypes.h"
#include "extobj.h"

ExternalObject extobj;

void read_extobj()
{
	ConfigFile cf;
	ConfigElem elem;

	if (FileExists( "config/extobj.local.cfg" ))
	{
		cf.open( "config/extobj.local.cfg" );
		cf.readraw( elem );
	}
	else if (FileExists( "config/extobj.cfg" ))
	{
		cf.open( "config/extobj.cfg" );
		cf.readraw( elem );
	}

	extobj.tillerman = elem.remove_unsigned("Tillerman", 0xF010);
	extobj.port_plank = elem.remove_unsigned("Port_Plank", 0xF011);
	extobj.starboard_plank = elem.remove_unsigned("Starboard_Plank", 0xF012);
	extobj.hold = elem.remove_unsigned("Hold", 0xF013);

	extobj.wrestling = elem.remove_unsigned("Wrestling", 0xF020);
	extobj.mount = elem.remove_unsigned("Mount", 0xF021);

	extobj.secure_trade_container = elem.remove_unsigned("Secure_Trade_Container", 0xFF01);
	extobj.wornitems_container = elem.remove_unsigned("Wornitems_Container", 0xFF02);
}
