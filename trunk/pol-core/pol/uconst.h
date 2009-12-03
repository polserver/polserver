/*
History
=======
2006/05/16 Shinigami: added URACE with RACE_* Constants
2009/10/12 Turley:    whisper/yell/say-range ssopt definition
2009/12/02 Turley:    added gargoyle race
2009/12/03 Turley:    added MOVEMODE_FLY

Notes
=======

*/

#ifndef __UCONST_H
#define __UCONST_H

enum UGENDER {
	GENDER_MALE		=	0,
	GENDER_FEMALE	=	1
};

enum URACE {
	RACE_HUMAN		=	0,
	RACE_ELF		=	1,
    RACE_GARGOYLE   =   2
};

/* Character Facings:
	N^
		7	0	1
		6	*	2
		5	4	3
*/

enum UFACING {
	FACING_N	=	0,
	FACING_NE	=	1,
	FACING_E	=	2,
	FACING_SE	=	3,
	FACING_S	=	4,
	FACING_SW	=	5,
	FACING_W	=	6,
	FACING_NW	=	7
};


enum UTEXTTYPE {
	TEXTTYPE_NORMAL				=	0x00,
	TEXTTYPE_SYS_BROADCAST		=	0x01,
	TEXTTYPE_EMOTE				=	0x02,
	TEXTTYPE_YOU_SEE			=	0x06,
	TEXTTYPE_WHISPER			=	0x08,
	TEXTTYPE_YELL				=	0x09
};


/* how to validate these values (and how range checking works on the client):
   tell the client about a character.  Take your character to the spot that
   character is standing on.  Walk straight, 18 steps in any direction.  Run back
   to the spot.  The character will still be there.  If you walk 19 steps, the
   client automatically removes the object - the server doesn't need to tell it to.
   Simple rectangular distance is used - if the x's differ by <= 18, and the y's differ
   by <= 18, you're in range.  If either is out of range, you're out.
*/
#define RANGE_VISUAL                    18
#define RANGE_VISUAL_LARGE_BUILDINGS    18 // 31?


const unsigned VALID_ITEM_COLOR_MASK = 0x0FFF;


enum MOVEMODE { 
    MOVEMODE_NONE   = 0,
    MOVEMODE_LAND   = 1, 
    MOVEMODE_SEA    = 2, 
    MOVEMODE_AIR    = 4,
    MOVEMODE_FLY    = 8
};

#define PLAYER_CHARACTER_HEIGHT		15

#endif /* UCONST_H */
