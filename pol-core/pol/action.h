/** @file
 *
 * @par History
 * - 2009/07/27 MuadDib:   Packet Struct Refactoring
 */


#ifndef ACTION_H
#define ACTION_H

#include <cstddef>
#include <fmt/format.h>
#include <vector>

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Mobile
{
class Character;
}
namespace Core
{
enum UACTION
{
  ACTION__LOWEST = 0x00,
  ACTION_WALK_UNARMED = 0x00,
  ACTION_WALK_ARMED = 0x01,
  ACTION_RUN1 = 0x02,
  ACTION_RUN2 = 0x03,
  ACTION_LOOK_AROUND = 0x05,
  ACTION_LOOK_DOWN = 0x06,
  ACTION_WARMODE1 = 0x07, /* hand down */
  ACTION_WARMODE2 = 0x08, /* hand up */

  ACTION_HUMAN_ATTACK = 0x9, /* hand to hand or sword, not bow */
  ACTION_HUMAN_ATACK_THRUST = 0x0A,
  ACTION_HUMAN_ATTACK_BIGSWING = 0x0B, /* big overhead swing */
  ACTION_ATTACK4 = 0x0C,               /* hmm, sort of up/down */
  ACTION_ATTACK5 = 0x0D,
  ACTION_ATTACK6 = 0x0E,
  ACTION_READY2 = 0x0F,
  ACTION_CAST_SPELL1 = 0x10,
  ACTION_CAST_SPELL2 = 0x11,
  ACTION_HUMAN_SHOOT_BOW = 0x12,
  ACTION_SHOOT_CROSSBOW = 0x13,
  ACTION_GOT_HIT = 0x14,
  ACTION_STRUCK_DOWN_BACKWARD = 0x15,
  ACTION_STRUCK_DOWN_FORWARD = 0x16,
  ACTION_RIDINGHORSE1 = 0x17,
  ACTION_RIDINGHORSE2 = 0x18,
  ACTION_RIDINGHORSE3 = 0x19,
  ACTION_RIDINGHORSE4 = 0x1A, /* a type of swing */
  ACTION_RIDINGHORSE5 = 0x1B, /* shoot bow */
  ACTION_RIDINGHORSE6 = 0x1C, /* shoot crossbow */
  ACTION_RIDINGHORSE7 = 0x1D,
  ACTION_DODGE1 = 0x1E, /* shield block? */
  ACTION_UNKNOWN1 = 0x1F,


  ACTION_BOW = 0x20,
  ACTION_SALUTE = 0x21,
  ACTION_EAT = 0x22,
  ACTION__HIGHEST = 0x22
};
inline auto format_as( UACTION a )
{
  return fmt::underlying( a );
}

inline bool UACTION_IS_VALID( unsigned short action )
{
  // don't need to check >= ACTION__LOWEST since it's 0.  avoids a warning.
  return ( action <= ACTION__HIGHEST );
}

enum DIRECTION_FLAG_OLD
{
  BACKWARD = 1,
  FORWARD = 0
};
enum REPEAT_FLAG_OLD
{
  REPEAT = 1,
  NOREPEAT = 0
};
void send_action_to_inrange( const Mobile::Character* obj, UACTION action,
                             unsigned short framecount = 0x05, unsigned short repeatcount = 0x01,
                             DIRECTION_FLAG_OLD backward = FORWARD,
                             REPEAT_FLAG_OLD repeatflag = NOREPEAT, unsigned char delay = 0x01 );

struct MobileTranslate
{
  struct OldAnimDef
  {
    bool valid;
    u16 action;
    u16 framecount;
    u16 repeatcount;
    u8 backward;
    u8 repeatflag;
    u8 delay;
    OldAnimDef();
  };
  struct NewAnimDef
  {
    bool valid;
    u16 anim;
    u16 action;
    u8 subaction;
    NewAnimDef();
  };
  std::vector<u16> graphics;
  OldAnimDef old_anim[ACTION__HIGHEST + 1];
  NewAnimDef new_anim[ACTION__HIGHEST + 1];
  bool supports_mount;
  MobileTranslate();
  bool has_graphic( u16 graphic ) const;
  size_t estimateSize() const;
};
}  // namespace Core
}  // namespace Pol
#endif
