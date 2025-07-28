/** @file
 *
 * @par History
 * - 2009/07/26 MuadDib:   updates for new Enum::Packet IDs
 * - 2009/12/02 Turley:    layer 15 is face layer
 */
#pragma once

#include <fmt/format.h>
#include <string>
#include <vector>

namespace Pol::Core
{
struct ArmorZone
{
  std::string name;
  double chance;
  std::vector<unsigned short> layers;
};

enum LAYER_INFO
{
  LOWEST_LAYER = 1,
  LAYER_EQUIP__LOWEST = 1,
  HIGHEST_LAYER = 25,
  NUM_LAYERS = 25,
  LAYER_EQUIP__HIGHEST = 25,
  LAYER_VENDOR_FOR_SALE = 26,
  LAYER_VENDOR_PLAYER_ITEMS = 27,
  LAYER_VENDOR_BUYABLE_ITEMS = 28,
  LAYER_BANKBOX = 29
};

inline auto format_as( LAYER_INFO l )
{
  return fmt::underlying( l );
}
// TODO: figure out which is left and right
enum LAYER_DETAILED
{
  LAYER_HAND1 = 1,
  LAYER_HAND2 = 2,
  LAYER_SHOES = 3,
  LAYER_PANTS = 4,
  LAYER_SHIRT = 5,
  LAYER_HAT = 6,
  LAYER_GLOVES = 7,
  LAYER_RING = 8,
  LAYER_TALISMAN = 9,
  LAYER_GORGET = 10,
  LAYER_HAIR = 11,
  LAYER_HALFAPRON = 12,
  LAYER_CHEST_ARMOR = 13,
  LAYER_BRACELET = 14,
  LAYER_FACE = 15,  // UO:KR/SA Face item
  LAYER_BEARD = 16,
  LAYER_TUNIC = 17,
  LAYER_EARRINGS = 18,
  LAYER_ARM_ARMOR = 19,
  LAYER_CLOAK = 20,
  LAYER_BACKPACK = 21,
  LAYER_ROBE_DRESS = 22,
  LAYER_SKIRT = 23,
  LAYER_LEG_ARMOR = 24,
  LAYER_MOUNT = 25
};
inline auto format_as( LAYER_DETAILED l )
{
  return fmt::underlying( l );
}

/*
layering.  'worn items' for a character seem to occupy different
'layer' locations.  I've seen the following:
0x01 1 dagger (right-hand?)
0x02 2 left hand - staffs, pitchfork, crook etc.
0x03 3 shoes
0x04 4 pants
0x05 5 shirt
0x06 6 hat
0x07 7 gloves
0x08 8 ring?
0x09 9 talismans
0x0A 10 gorget
0x0B 11 hair
0x0C 12 half-apron
0x0D 13 (armor)tunic/chest armor, belt pouch (?)
0x0E 14 bracelet
0x0F 15 backpack (wtf? the "real" backpack sits in layer 21)
0x10 16 beard
0x11 17 full apron, body sash, doublet, jester suit, tunic, surcoat
0x12 18 earrings
0x13 19 arms
0x14 20 cloak
0x15 21 backpack
0x16 22 dresses, robes
0x17 23 skirt, kilt
0x18 24 platemail legs
0x19 25 horse?
0x1A 26 Buy Container
0x1B 27 Resale Container
0x1C 28 Sell Container
0x1D 29 Bankbox
0x1E 30 unknown
0x1F 31 unknown



I assume that there can only be a single item in each layer.
Note that for the most part, the code doesn't need to know this.  The
data is in tiledata.mul (we don't check this yet), and the client
makes sure to drop items only in the correct layer (again, server
needs to check this).  The only spcial case is the backpack -
Character::backpack() uses LAYER_BACKPACK to return the backpack item.
*/
}  // namespace Pol::Core
