/** @file
 *
 * @par History
 * - 2006/05/16 Shinigami: added URACE with RACE_* Constants
 * - 2009/10/12 Turley:    whisper/yell/say-range ssopt definition
 * - 2009/12/02 Turley:    added gargoyle race
 * - 2009/12/03 Turley:    added MOVEMODE_FLY
 */


#ifndef __UCONST_H
#define __UCONST_H

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Plib
{
enum UGENDER : u8
{
  GENDER_MALE = 0,
  GENDER_FEMALE = 1
};

enum URACE : u8
{
  RACE_HUMAN = 0,
  RACE_ELF = 1,
  RACE_GARGOYLE = 2
};


enum UTEXTTYPE : u8
{
  TEXTTYPE_NORMAL = 0x00,
  TEXTTYPE_SYS_BROADCAST = 0x01,
  TEXTTYPE_EMOTE = 0x02,
  TEXTTYPE_YOU_SEE = 0x06,
  TEXTTYPE_WHISPER = 0x08,
  TEXTTYPE_YELL = 0x09
};


const unsigned VALID_ITEM_COLOR_MASK = 0x0FFF;


enum MOVEMODE : u8
{
  MOVEMODE_NONE = 0,
  MOVEMODE_LAND = 1,
  MOVEMODE_SEA = 2,
  MOVEMODE_AIR = 4,
  MOVEMODE_FLY = 8
};

#define MAXBUFFER 2560

#define MAX_CONTAINER_ITEMS 3200  // 0x3C packet maxsize 5+3200*20 ~= 0xFFFF
#define MAX_SLOTS 255

#define MAX_STACK_ITEMS 60000L
#define MAX_WEIGHT 65535L

const unsigned short DEFAULT_TEXT_FONT = 3;
const unsigned short DEFAULT_TEXT_COLOR = 0x3B2;
const unsigned short DEFAULT_TEXT_REQUIREDCMD = 0;
}  // namespace Plib
}  // namespace Pol
#endif /* UCONST_H */
