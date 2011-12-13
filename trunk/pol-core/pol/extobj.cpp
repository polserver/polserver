/*
History
=======
2011/11/10 Tomi:	Added file
2011/12/13 Tomi:    Added rope, wheel, hull, sails, tiller, rudder, storage and weaponslot objtypes for new boat support
					Changed default values from 0xFXXX to 0x1FXXX

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

	extobj.tillerman = elem.remove_unsigned("Tillerman", 0x1F010);
	extobj.port_plank = elem.remove_unsigned("Port_Plank", 0x1F011);
	extobj.starboard_plank = elem.remove_unsigned("Starboard_Plank", 0x1F012);
	extobj.hold = elem.remove_unsigned("Hold", 0x1F013);
	extobj.rope = elem.remove_unsigned("Rope", 0x1F014);
	extobj.wheel = elem.remove_unsigned("Wheel", 0x1F015);
	extobj.hull = elem.remove_unsigned("Hull", 0x1F016);
	extobj.tiller = elem.remove_unsigned("Tiller", 0x1F017);
	extobj.rudder = elem.remove_unsigned("Rudder", 0x1F018);
	extobj.sails = elem.remove_unsigned("Sails", 0x1F019);
	extobj.storage = elem.remove_unsigned("Storage", 0x1F020);
	extobj.weaponslot = elem.remove_unsigned("Weaponslot", 0x1F021);

	extobj.wrestling = elem.remove_unsigned("Wrestling", 0x1F020);
	extobj.mount = elem.remove_unsigned("Mount", 0x1F021);

	extobj.secure_trade_container = elem.remove_unsigned("Secure_Trade_Container", 0x1FF01);
	extobj.wornitems_container = elem.remove_unsigned("Wornitems_Container", 0x1FF02);
}
