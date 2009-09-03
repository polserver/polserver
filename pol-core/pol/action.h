/*
History
=======
2009/07/27 MuadDib:   Packet Struct Refactoring

Notes
=======

*/


#ifndef ACTION_H
#define ACTION_H

enum UACTION 
{
    ACTION__LOWEST          = 0x00,
    ACTION_WALK_UNARMED     = 0x00,
    ACTION_WALK_ARMED       = 0x01,
    ACTION_RUN1             = 0x02,
    ACTION_RUN2             = 0x03,
    ACTION_LOOK_AROUND      = 0x05,
    ACTION_LOOK_DOWN        = 0x06,
    ACTION_WARMODE1         = 0x07, /* hand down */
    ACTION_WARMODE2         = 0x08, /* hand up */

    ACTION_HUMAN_ATTACK     = 0x9, /* hand to hand or sword, not bow */
    ACTION_HUMAN_ATACK_THRUST       = 0x0A,
    ACTION_HUMAN_ATTACK_BIGSWING    = 0x0B, /* big overhead swing */
    ACTION_ATTACK4          = 0x0C, /* hmm, sort of up/down */
    ACTION_ATTACK5          = 0x0D,
    ACTION_ATTACK6          = 0x0E,
    ACTION_READY2           = 0x0F,
    ACTION_CAST_SPELL1      = 0x10,
    ACTION_CAST_SPELL2      = 0x11,
    ACTION_HUMAN_SHOOT_BOW  = 0x12,
    ACTION_SHOOT_CROSSBOW   = 0x13,
    ACTION_GOT_HIT          = 0x14,
    ACTION_STRUCK_DOWN_BACKWARD     = 0x15,
    ACTION_STRUCK_DOWN_FORWARD      = 0x16,
    ACTION_RIDINGHORSE1     = 0x17,
    ACTION_RIDINGHORSE2     = 0x18,
    ACTION_RIDINGHORSE3     = 0x19,
    ACTION_RIDINGHORSE4     = 0x1A, /* a type of swing */
    ACTION_RIDINGHORSE5     = 0x1B, /* shoot bow */
    ACTION_RIDINGHORSE6     = 0x1C, /* shoot crossbow */
    ACTION_RIDINGHORSE7     = 0x1D,
    ACTION_DODGE1           = 0x1E, /* shield block? */
    ACTION_UNKNOWN1         = 0x1F,
    

	ACTION_BOW		        = 0x20,
	ACTION_SALUTE	        = 0x21,
    ACTION_EAT              = 0x22,
    ACTION__HIGHEST         = 0x22
};

inline bool UACTION_IS_VALID( unsigned short action )
{
    // don't need to check >= ACTION__LOWEST since it's 0.  avoids a warning.
    return (action <= ACTION__HIGHEST);
}
class Character;

#include "pktout.h"
void send_action_to_inrange( const Character* obj, UACTION action,
							unsigned short framecount = 0x05,
							unsigned short repeatcount = 0x01,
							PKTOUT_6E::DIRECTION_FLAG backward = PKTOUT_6E::FORWARD,
							PKTOUT_6E::REPEAT_FLAG repeatflag = PKTOUT_6E::NOREPEAT,
							unsigned char delay = 0x01 );

#endif
