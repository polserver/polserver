/** @file
 *
 * @par History
 */


#ifndef PLIB_MAPCELL_H
#define PLIB_MAPCELL_H


namespace Pol::Plib
{
struct FLAG
{
  enum
  {
    NONE = 0x0000,
    MOVELAND = 0x0001,     // 's'
    MOVESEA = 0x0002,      // 'n'
    BLOCKSIGHT = 0x0004,   // 'L'
    OVERFLIGHT = 0x0008,   // 'f'
    ALLOWDROPON = 0x0010,  // 'd'
    GRADUAL = 0x0020,      // 'g'
    BLOCKING = 0x0040,     // 'B'
    // MORE_SOLIDS:
    // for a map entry, indicates there are entries in the solids file.
    // for a solid, indicates there is another solid following
    MORE_SOLIDS = 0x0080,  // '+'

    // the flags above go into base.dat, solids.dat etc, so have to be the lower 8 bits.

    // the flags below are read from tiles.cfg, and don't necessaryily need to stay in here.
    STACKABLE = 0x0100,
    MOVABLE = 0x0200,
    // 0x0400
    EQUIPPABLE = 0x0800,
    DESC_PREPEND_A = 0x1000,
    DESC_PREPEND_AN = 0x2000,

    WALKBLOCK = MOVELAND | MOVESEA | BLOCKING | BLOCKSIGHT | ALLOWDROPON,
    MOVE_FLAGS = MOVELAND | MOVESEA | BLOCKING | GRADUAL,
    DROP_FLAGS = BLOCKING | ALLOWDROPON,
    ALL = 0xFFFFFFFFLu
  };
};

struct MAPCELL
{
  signed char z;
  unsigned char flags;
};
static_assert( sizeof( MAPCELL ) == 2, "size missmatch" );
}  // namespace Pol::Plib

#endif
