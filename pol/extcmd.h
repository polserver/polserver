/*
History
=======


Notes
=======

*/

#ifndef __EXTCMD_H
#define __EXTCMD_H

class Client;
struct PKTIN_12;

// Extended messages come in the form of a null-terminated string.  The
// "Extended Message" message handler validates that the string is in 
// fact null terminated where it's supposed to be.  It does not currently
// verify that the string isn't terminated early also, or that the string
// only contains printables.

enum UEXTMSGID
{
	EXTMSGID_SKILL		=	0x24,	// Form: "SkillId parameter" (SkillId 0-based, param usu. 0)
	EXTMSGID_CASTSPELL1 =	0x27,	// Form: "SpellId SpellBookSerial"
	EXTMSGID_SPELLBOOK  =   0x43,	// Form: (empty)
	EXTMSGID_CASTSPELL2	=	0x56,	// Form: "SpellId"
	EXTMSGID_OPENDOOR	=	0x58,	// Form: (empty)
	EXTMSGID_ACTION		=	0xC7	// Form: "action_name"
};

class ExtendedMessageHandler
{
public:
	ExtendedMessageHandler( UEXTMSGID submsgtype,
		                    void (*func)(Client *client, PKTIN_12 *msg) );
};

#endif
