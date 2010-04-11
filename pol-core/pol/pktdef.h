/*
History
=======
2009/07/24 MuadDib:   Initial creation.
2010/01/22 Turley:    Speedhack Prevention System

Notes
=======
This is for all general packet #define/enum that don't fit anywhere else or go to multiple types of packets.

*/

#ifndef __PKTDEFH
#define __PKTDEFH

#define LAYER_VENDOR_FOR_SALE	  0x1A
#define LAYER_VENDOR_PLAYER_ITEMS  0x1B
#define LAYER_VENDOR_BUYABLE_ITEMS 0x1C

#define ITEM_FLAG_FORCE_MOVABLE	0x20
#define ITEM_FLAG_HIDDEN		0x80
#define SPEECH_MAX_LEN			200

#define MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP       0
#define MOVE_ITEM_FAILURE_TOO_FAR_AWAY              1
#define MOVE_ITEM_FAILURE_OUT_OF_SIGHT              2
#define MOVE_ITEM_FAILURE_BELONGS_TO_OTHER          3
#define MOVE_ITEM_FAILURE_ALREADY_HOLDING_AN_ITEM   4
#define MOVE_ITEM_FAILURE_ALREADY_WORN              5
#define MOVE_ITEM_FAILURE_ILLEGAL_EQUIP             5

// none of the below are right, BTW.
#define MOVE_ITEM_FAILURE_UNKNOWN			5
#define MOVE_ITEM_FAILURE_ALREADY_PICKED_UP 5
#define MOVE_ITEM_FAILURE_ALREADY_CARRYING	5

#define MSGOPT_33_FLOW_PAUSE	1
#define MSGOPT_33_FLOW_RESTART	0


#define EFFECT_TYPE_MOVING          0x00
#define EFFECT_TYPE_LIGHTNING_BOLT  0x01 
#define EFFECT_TYPE_STATIONARY_XYZ  0x02
#define EFFECT_TYPE_FOLLOW_OBJ      0x03 

#define SPELL_EFFECT_FIREBALL   0x36D4
#define SPELL_EFFECT_EBOLT      0x379F

#define DIR_FLAG_RUN 0x80

/* Notes on 'dir': bit 0x80 is 'run' bit.  Mask 0x78 must be left zero or the directions
will display wrong.  Only mask 0x7 is used, for facing. 
*/

#define CHAR_HILITE_INNOCENT    0x01
#define CHAR_HILITE_FRIEND      0x02
#define CHAR_HILITE_ATTACKABLE  0x03
#define CHAR_HILITE_CRIMINAL    0x04
#define CHAR_HILITE_ENEMY       0x05
#define CHAR_HILITE_MURDERER    0x06
#define CHAR_HILITE_INVUL       0x07

#define LOGIN_ERROR_NO_ACCOUNT       0x00
#define LOGIN_ERROR_OTHER_CHAR_INUSE 0x01
#define LOGIN_ERROR_ACCOUNT_BLOCKED  0x02
#define LOGIN_ERROR_WRONG_PASSWORD   0x03
// 04 "some problem communicating with Origin.  Please restart Ultima Online and try again."
// 05 same
#define LOGIN_ERROR_MISC			0x06
#define LOGIN_ERROR_IDLE			0xFE
#define LOGIN_ERROR_COMMS			0xFF

#define URL_MAX_LEN 200

#define PKTIN_02_FACING_MASK 0x07
#define PKTIN_02_DIR_RUNNING_BIT 0x80
#define PKTIN_02_SIZE 0x07
#define PKTIN_02_MOUNT_RUN   80
#define PKTIN_02_MOUNT_WALK 180
#define PKTIN_02_FOOT_RUN   180
#define PKTIN_02_FOOT_WALK  380
#define PKTIN_02_ASYNCHRONOUS 400

#define STATTYPE_STATWINDOW     4
#define STATTYPE_SKILLWINDOW    5

#define RESURRECT_CHOICE_SELECT         0
#define RESURRECT_CHOICE_INSTARES       1
#define RESURRECT_CHOICE_GHOST          2

#define MSG71_SUBCMD_2_CONSTANT {0x01,0x91,0x84,0x0A,0x06,0x1E,0xFD,0x01,0x0B,0x15,0x2E,0x01,0x0B,0x17,0x0B,0x01,0xBB,0x20,0x46,0x04,0x66,0x13,0xF8,0x00,0x00,0x0E,0x75,0x00,0x00}

// Changed 1/9/2007 MuadDib per OSI packet logs.
const u8 CHAR_FLAG1_INVISIBLE   = 0x80;
const u8 CHAR_FLAG1_WARMODE     = 0x40;
const u8 CHAR_FLAG1_YELLOWHEALTH= 0x08;
const u8 CHAR_FLAG1_POISONED    = 0x04;
const u8 CHAR_FLAG1_FLYING      = 0x04;  //since client 7.0
const u8 CHAR_FLAG1_CANALTER    = 0x02;  //only Paperdoll packet
const u8 CHAR_FLAG1_GENDER      = 0x02;
const u8 CHAR_FLAG1_NORMAL      = 0x00;

#define RESPONSE_MAX_LEN 200

#define PKTOUT_53_WARN_NO_CHARACTER       1
#define PKTOUT_53_WARN_CHARACTER_EXISTS   2
#define PKTOUT_53_WARN_CANT_CONNECT_1     3
#define PKTOUT_53_WARN_CANT_CONNECT_2     4
#define PKTOUT_53_WARN_CHARACTER_IN_WORLD 5
#define PKTOUT_53_WARN_LOGIN_PROBLEM      6
#define PKTOUT_53_WARN_CHARACTER_IDLE     7
#define PKTOUT_53_WARN_CANT_CONNECT_3     8

#define PKTOUT_54_FLAG_REPEAT     0
#define PKTOUT_54_FLAG_SINGLEPLAY 1

#define PKTOUT_65_TYPE_NONE 0
#define PKTOUT_65_TYPE_RAIN 1
#define PKTOUT_65_TYPE_SNOW 2
#define PKTOUT_65_WTYPE_START_TO_RAIN  0x00
#define PKTOUT_65_WTYPE_FIERCE_STORM   0x01
#define PKTOUT_65_WTYPE_BEGIN_TO_SNOW  0x02
#define PKTOUT_65_WTYPE_STORM_BREWING  0x03
#define PKTOUT_65_WTYPE_STOP_WEATHER   0xFF

struct PKTOUT_6E
{
	enum DIRECTION_FLAG { BACKWARD = 1, FORWARD = 0 };
	enum REPEAT_FLAG { REPEAT = 1, NOREPEAT = 0 };
};

#define PKTOUT_A6_TYPE_TIP    0
#define PKTOUT_A6_TYPE_UPDATE 1

struct PKTOUT_A9
{
	enum {
		FLAG_UNK_0x0001             = 0x0001, // ?
		FLAG_SEND_CONFIG_REQ_LOGOUT = 0x0002, // Sends Configuration to Server
		FLAG_SINGLE_CHARACTER_SIEGE = 0x0004, // ?
		FLAG_ENABLE_NPC_POPUP_MENUS = 0x0008, // Enable NPC PopUp Menus
		FLAG_SINGLE_CHARACTER       = 0x0010, // Use 1 Character only
		FLAG_AOS_FEATURES           = 0x0020, // Age of Shadows
		FLAG_UPTO_SIX_CHARACTERS    = 0x0040, // Use up to 6 Characters (not only 5)
		FLAG_SE_FEATURES            = 0x0080, // Samurai Empire
		FLAG_ML_FEATURES            = 0x0100, // Mondain's Legacy
		FLAG_SEND_UO3D_TYPE         = 0x0400, // KR and UO:SA will send 0xE1 packet
		FLAG_UNK_0x0800             = 0x0800, // ?
		FLAG_UPTO_SEVEN_CHARACTERS  = 0x1000, // Use up to 7 Characters
		FLAG_UNK_0x2000             = 0x2000  // ?
	};
};
#define PKTOUT_B7_MAX_CHARACTERS 256

#define PKTOUT_BA_ARROW_OFF 0
#define PKTOUT_BA_ARROW_ON  0

struct PKTOUT_BC
{
	enum { SEASON_SPRING = 0, SEASON_SUMMER, SEASON_FALL, SEASON_WINTER, SEASON_DESOLATION };
	enum { PLAYSOUND_NO = 0, PLAYSOUND_YES = 1 };
};

struct PKTOUT_C0
{
	enum {
		EFFECT_MOVING  = 0x00,
		EFFECT_LIGHTNING = 0x01,
		EFFECT_FIXEDXYZ = 0x02,
		EFFECT_FIXEDFROM = 0x03
	};
};

#endif

