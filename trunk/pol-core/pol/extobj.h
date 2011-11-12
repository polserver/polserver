/*
History
=======
2011/11/10 Tomi:	Added file

Notes
=======

*/

#ifndef EXTOBJ_H
#define EXTOBJ_H

struct ExternalObject {
	unsigned int tillerman;
	unsigned int port_plank;
	unsigned int starboard_plank;
	unsigned int hold;

	unsigned int wrestling;
	unsigned int mount;

	unsigned int secure_trade_container;
	unsigned int wornitems_container;
};

extern ExternalObject extobj;

void read_extobj();

#endif
