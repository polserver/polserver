/*
History
=======
2006/05/16 Shinigami: UOBJ_*MALE_GHOST renamed to UOBJ_HUMAN_*MALE_GHOST
                      added UOBJ_ELF_* constants
2009/09/03 MuadDib:	  Changes for account related source file relocation
					  Changes for multi related source file relocation
2009/12/02 Turley:    Gargoyle support

Notes
=======

*/

#ifndef __UOBJ_H
#define __UOBJ_H

#define UOBJ_HSA_MAX			0xFFFF
#define UOBJ_SA_MAX				0x7FFF
#define UOBJ_DEFAULT_MAX		0x3FFF

#define GRAPHIC_NODRAW          0x0001


#define UOBJ_OGRE                0x0001
#define UOBJ_ETTIN               0x0002
#define UOBJ_ZOMBIE              0x0003
#define UOBJ_GARGOYLE            0x0004
#define UOBJ_EAGLE               0x0005
#define UOBJ_BIRD                0x0006
#define UOBJ_ARMED_ORC           0x0007
#define UOBJ_CORPSER             0x0008
#define UOBJ_DAEMON              0x0009
#define UOBJ_DAEMON_W_SWORD      0x000A

#define UOBJ_GREEN_DRAGON        0x000C
#define UOBJ_AIR_ELEMENTAL       0x000D
#define UOBJ_EARTH_ELEMENTAL     0x000E
#define UOBJ_FIRE_ELEMENTAL      0x000F
#define UOBJ_WATER_ELEMENTAL     0x0010
#define UOBJ_ORC                 0x0011
#define UOBJ_ETTIN_W_AXE         0x0012

#define UOBJ_GIANT_SNAKE         0x0015
#define UOBJ_GAZER               0x0016

#define UOBJ_LICHE               0x0018

#define UOBJ_FLOAT_SKELETON      0x001A

#define UOBJ_GIANT_SPIDER        0x001C
#define UOBJ_GORILLA             0x001D
#define UOBJ_HARPY               0x001E
#define UOBJ_HEADLESS            0x001F

#define UOBJ_LIZARDMAN           0x0021

#define UOBJ_LIZARDMAN_W_SPEAR   0x0023
#define UOBJ_LIZARDMAN_W_HAMMER  0x0024

#define UOBJ_MONGBAT             0x0027

#define UOBJ_ORC_W_CLUB          0x0029
#define UOBJ_RATMAN              0x002A

#define UOBJ_RATMAN_W_CLUB       0x002C
#define UOBJ_RATMAN_W_SWORD      0x002D

#define UOBJ_REAPER              0x002F
#define UOBJ_GIANT_SCORPION      0x0030

#define UOBJ_SKELETON            0x0032
#define UOBJ_SLIME               0x0033
#define UOBJ_SNAKE               0x0034
#define UOBJ_TROLL               0x0035
#define UOBJ_TROLL2              0x0036
#define UOBJ_TROLL3              0x0037
#define UOBJ_SKELETON_W_AXE      0x0038
#define UOBJ_SKELETON_W_SWORD    0x0039
#define UOBJ_WISP                0x003A
#define UOBJ_RED_DRAGON          0x003B
#define UOBJ_SMALL_GREEN_DRAGON  0x003C
#define UOBJ_SMALL_RED_DRAGON    0x003D
 
#define UOBJ_SEA_SERPENT         0x0096
#define UOBJ_DOLPHIN             0x0097

#define UOBJ_WHITE_HORSE         0x00C8
#define UOBJ_CAT                 0x00C9
#define UOBJ_ALLIGATOR           0x00CA
#define UOBJ_SMALL_PIG           0x00CB
#define UOBJ_BROWN_HORSE         0x00CC
#define UOBJ_RABBIT              0x00CD
#define UOBJ_SHEEP               0x00CF
#define UOBJ_CHICKEN             0x00D0
#define UOBJ_GOAT                0x00D1

#define UOBJ_BROWN_BEAR          0x00D3
#define UOBJ_GRIZZLY_BEAR        0x00D4
#define UOBJ_POLAR_BEAR          0x00D5
#define UOBJ_PANTHER             0x00D6
#define UOBJ_GIANT_RAT           0x00D7
#define UOBJ_COW                 0x00D8
#define UOBJ_DOG                 0x00D9

#define UOBJ_LLAMA               0x00DC
#define UOBJ_WALRUS              0x00DD

#define UOBJ_LAMB                0x00DF

#define UOBJ_JACKAL              0x00E1
#define UOBJ_HORSE2              0x00E2

#define UOBJ_HORSE3              0x00E4

#define UOBJ_BROWN_COW           0x00E7
#define UOBJ_BULL                0x00E8
#define UOBJ_BW_COW              0x00E9
#define UOBJ_DEER                0x00EA

#define UOBJ_SMALL_DEER          0x00ED
#define UOBJ_RAT                 0x00EE

#define UOBJ_LARGE_PIG           0x0122
#define UOBJ_PACK_HORSE_W_BAGS   0x0123
#define UOBJ_PACK_LLAMA_W_BAGS   0x0124

#define UOBJ_HUMAN_MALE          0x0190
#define UOBJ_HUMAN_FEMALE        0x0191
#define UOBJ_HUMAN_MALE_GHOST    0x0192
#define UOBJ_HUMAN_FEMALE_GHOST  0x0193

#define UOBJ_ELF_MALE            0x025D
#define UOBJ_ELF_FEMALE          0x025E
#define UOBJ_ELF_MALE_GHOST      0x025F
#define UOBJ_ELF_FEMALE_GHOST    0x0260

#define UOBJ_GARGOYLE_MALE         0x029A
#define UOBJ_GARGOYLE_FEMALE       0x029B
#define UOBJ_GARGOYLE_MALE_GHOST   0x02B6
#define UOBJ_GARGOYLE_FEMALE_GHOST 0x02B7

#define UOBJ_HAT01               0x0194
#define UOBJ_HAT02               0x0195
#define UOBJ_HAT03               0x0196
#define UOBJ_HAT04               0x0197
#define UOBJ_HAT05               0x0198
#define UOBJ_HAT06               0x0199
#define UOBJ_HAT07               0x019A
#define UOBJ_HAT08               0x019B
#define UOBJ_HAT09               0x019C
#define UOBJ_HAT10               0x019D
#define UOBJ_BEAR_HAT            0x019E
#define UOBJ_DEER_HAT            0x019F
#define UOBJ_ORC_MASK            0x01A0
#define UOBJ_XMAS_TREE_HAT       0x01A1
#define UOBJ_VOODOO_MASK         0x01A2
#define UOBJ_SHORT_HAIR          0x01A3
#define UOBJ_MALE_KID_BLONDE     0x01A4
#define UOBJ_MALE_KID_BRUNETTE   0x01A5
#define UOBJ_FEMALE KID_BLOND    0x01A6
#define UOBJ_FEMALE_KID_BRUNETTE 0x01A7
#define UOBJ_INVISIBLE_MAN       0x01A8

#define UOBJ_TROUSERS_SHORT      0x01AE
#define UOBJ_TROUSERS_LONG       0x01AF

#define UOBJ_FLOATING_SHIRT      0x01b2 
#define UOBJ_FLOATING_SUIT       0x01b3 

#define UOBJ_WALKING_DRESS       0x01bf 
#define UOBJ_WALKING_FANCY_DRESS 0x01c0
 
#define UOBJ_WALKING_KILT        0x01c7 

#define UOBJ_WALKING_FULL_APRON  0x01d1 
#define UOBJ_WALKING_HALF_APRON  0x01d2 
/*
01 f4 - Lit Lantern
01 f5 - Lit Torch
01 f6 - Lit Candle
01 f7 - Lantern
01 f8 - Torch
01 f9 - Candle
02 09 - Leather Armor
03 90 - Jesters Suit
03 ca - Death Robe
03 db - Game Master Robe
03 de - Empty Lord British Armor
03 df - Blackthorn
03 e0 - Order Shield
03 e1 - Chaos Shield
*/

#define UOBJ_MAP1                   0x14EB
#define UOBJ_MAP2                   0x14EC
#define UOBJ_ROLLED_MAP1            0x14ED
#define UOBJ_ROLLED_MAP2            0x14EE

#define UOBJ_SCROLL__LOWEST			0x1F2D
#define UOBJ_SCROLL__HIGHEST		0x1F6C

#define UOBJ_GAMEMASTER			 0x03DB

#define UOBJ_BACKPACK				0x0E75
#define UOBJ_BARREL					0x0E77
#define UOBJ_SILVER_CHEST			0x0E7C
#define UOBJ_BRASS_BOX				0x0E80


#define UOBJ_MORTAR_AND_PESTLE		0x0E9B
#define UOBJ_GOLD_COIN				0x0EED
#define UOBJ_SPELLBOOK				0x0EFA

// Weapons
#define UOBJ_LONGSWORD              0x0F5E
#define UOBJ_WAR_AXE                0x13B0


#define UOBJ_DEATH_ROBE             0x1F03
#define UOBJ_CORPSE					0x2006
#define UOBJ_DEATH_SHROUD			0x204E




#define UOBJ_ITEM__LOWEST           0x0002

// Define the Default ( also minimun max_objtype allowed ) pol.cfg now uses max_objtype to let people define the maximum
#define EXTOBJ_HIGHEST_DEFAULT		0x20000


#endif
