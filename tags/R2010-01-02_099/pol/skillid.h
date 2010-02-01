/*
History
=======
2003/03/15 Dave: support configurable max skillid

Notes
=======

*/

#ifndef SKILLID_H
#define SKILLID_H

#include "uoclient.h"

enum USKILLID {
	SKILLID__LOWEST		= 0,
	SKILLID__CLIENT_LOWEST		= 0,
	SKILLID_ALCHEMY				= 0,
	SKILLID_ANATOMY				= 1,
    SKILLID_ANIMALLORE			= 2,
    SKILLID_ITEMID				= 3,
    SKILLID_ARMSLORE			= 4,
    SKILLID_BATTLE_DEFENSE  	= 5,
    SKILLID_BEGGING				= 6,
    SKILLID_BLACKSMITHY 		= 7,
    SKILLID_BOWCRAFT			= 8,
    SKILLID_PEACEMAKING			= 9,
    SKILLID_CAMPING				= 10,
    SKILLID_CARPENTRY			= 11,
    SKILLID_CARTOGRAPHY			= 12,
    SKILLID_COOKING				= 13,
    SKILLID_DETECTINGHIDDEN		= 14,
    SKILLID_ENTICEMENT			= 15,
    SKILLID_EVALUATINGINTEL		= 16,
    SKILLID_HEALING				= 17,
    SKILLID_FISHING				= 18,
    SKILLID_FORENSICS			= 19,
    SKILLID_HERDING				= 20,
    SKILLID_HIDING				= 21,
    SKILLID_PROVOCATION			= 22,
    SKILLID_INSCRIPTION			= 23,
    SKILLID_LOCKPICKING			= 24,
    SKILLID_MAGERY				= 25,
    SKILLID_MAGICRESISTANCE		= 26,
    SKILLID_TACTICS				= 27,
    SKILLID_SNOOPING			= 28,
    SKILLID_MUSICIANSHIP		= 29,
    SKILLID_POISONING			= 30,
    SKILLID_ARCHERY				= 31,
    SKILLID_SPIRITSPEAK			= 32,
    SKILLID_STEALING			= 33,
    SKILLID_TAILORING			= 34,
    SKILLID_TAMING				= 35,
    SKILLID_TASTEID				= 36,
    SKILLID_TINKERING			= 37,
    SKILLID_TRACKING			= 38,
    SKILLID_VETERINARY			= 39,
    SKILLID_SWORDSMANSHIP		= 40,
    SKILLID_MACEFIGHTING		= 41,
    SKILLID_FENCING				= 42,
    SKILLID_WRESTLING			= 43,
    SKILLID_LUMBERJACKING		= 44,
    SKILLID_MINING				= 45,
    SKILLID_MEDITATION          = 46,
    SKILLID_STEALTH             = 47,
    SKILLID_REMOVE_TRAP         = 48,
	SKILLID_NECROMANCY			= 49,
	SKILLID_BATTLE_FOCUS		= 50,
	SKILLID_CHIVALRY			= 51,

	SKILLID__CLIENT_HIGHEST  	= 51,
	
	// additional skills, that the client can't directly know about, go here.
    SKILLID__HIGHEST            = 51,
	SKILLID__COUNT // use for array sizes, etc.
};

inline bool USKILLID_IS_VALID( unsigned short skillid )
{
    return (skillid <= uoclient_general.maxskills);
}
#endif
