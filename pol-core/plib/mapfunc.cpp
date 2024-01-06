/** @file
 *
 * @par History
 * - 2006/04/09 Shinigami: added uoconvert.cfg flag ShowRoofAndPlatformWarning
 * - 2006/05/30 Shinigami: fixed a bug with ShowRoofAndPlatformWarning - a missing expression
 * - 2009/12/03 Turley:    added gargoyle flying flag (hoverover/overflight flag)
 */

#include "mapfunc.h"

#include "../clib/bitutil.h"
#include "../clib/cfgelem.h"
#include "../clib/logfacility.h"
#include "mapcell.h"
#include "ustruct.h"

namespace Pol
{
namespace Plib
{
bool cfg_show_roof_and_platform_warning = true;
std::string flagstr( unsigned int flags )
{
  std::string tmp;
  if ( flags & FLAG::MOVELAND )
    tmp += "L";
  if ( flags & FLAG::MOVESEA )
    tmp += "S";
  if ( flags & FLAG::BLOCKSIGHT )
    tmp += "V";
  if ( flags & FLAG::OVERFLIGHT )
    tmp += "f";
  if ( flags & FLAG::ALLOWDROPON )
    tmp += "d";
  if ( flags & FLAG::GRADUAL )
    tmp += "g";
  if ( flags & FLAG::MORE_SOLIDS )
    tmp += "+";
  if ( flags & FLAG::BLOCKING )
    tmp += "B";
  if ( flags & FLAG::STACKABLE )
    tmp += "S";
  if ( flags & FLAG::MOVABLE )
    tmp += "M";
  // if (flags & FLAG::DOOR)
  //    tmp += "D";
  if ( flags & FLAG::EQUIPPABLE )
    tmp += "E";
  return tmp;
}
std::string flagdescs()
{
  static std::string out =
      "L=move land, S=move sea, V=block sight, f=overflight, d=allow drop, g=gradual, +=more "
      "solids, B=blocking, S=stackable, M=movable, E=equippable";
  return out;
}

u32 polflags_from_tileflags( unsigned short tile, u32 uoflags, bool use_no_shoot,
                             bool LOS_through_windows )
{
  u32 mapflags = 0;

  if ( ( uoflags & USTRUCT_TILE::FLAG_LIQUID ) && ( uoflags & USTRUCT_TILE::FLAG_FLOOR ) )
  {
    mapflags |= FLAG::MOVESEA;
  }
  else if ( ( uoflags & USTRUCT_TILE::FLAG_PLATFORM ) &&
            ( ~uoflags & USTRUCT_TILE::FLAG_BLOCKING ) )
  {
    mapflags |= FLAG::MOVELAND;
  }

  // GRADUAL only makes sense if also STANDABLE.
  if ( ( mapflags & FLAG::MOVELAND ) && ( uoflags & USTRUCT_TILE::FLAG_HALF_HEIGHT ) )
  {
    mapflags |= FLAG::GRADUAL;
  }

  if ( uoflags & USTRUCT_TILE::FLAG_HOVEROVER )
  {
    mapflags |= FLAG::OVERFLIGHT;
  }

  if ( uoflags & USTRUCT_TILE::FLAG_PLATFORM )
  {
    mapflags |= FLAG::ALLOWDROPON;
  }

  if ( uoflags & USTRUCT_TILE::FLAG_STACKABLE )
  {
    mapflags |= FLAG::STACKABLE;
  }

  if ( uoflags & USTRUCT_TILE::FLAG_EQUIPPABLE )
  {
    mapflags |= FLAG::EQUIPPABLE;
  }

  if ( uoflags & USTRUCT_TILE::FLAG_DESC_NEEDS_A )
  {
    mapflags |= FLAG::DESC_PREPEND_A;
  }

  if ( uoflags & USTRUCT_TILE::FLAG_DESC_NEEDS_AN )
  {
    mapflags |= FLAG::DESC_PREPEND_AN;
  }

  if ( use_no_shoot )

  {
    if ( ( uoflags & USTRUCT_TILE::FLAG_WALL ) &&

         ( uoflags & ( USTRUCT_TILE::FLAG_BLOCKING | USTRUCT_TILE::FLAG_DOOR ) )

    )

      mapflags |= FLAG::BLOCKSIGHT;


    if ( uoflags & USTRUCT_TILE::FLAG_NO_SHOOT )

      mapflags |= FLAG::BLOCKSIGHT;


    if ( !LOS_through_windows )

    {
      if ( ( uoflags & USTRUCT_TILE::FLAG_WINDOW ) &&

           ( uoflags & USTRUCT_TILE::FLAG_WALL )

      )

        mapflags |= FLAG::BLOCKSIGHT;
    }
    else

    {
      if ( uoflags & USTRUCT_TILE::FLAG_WINDOW )

        mapflags &= ~FLAG::BLOCKSIGHT;
    }
  }

  else

  {
    if ( ( uoflags & USTRUCT_TILE::FLAG_WALL ) && ( ~uoflags & USTRUCT_TILE::FLAG_BLOCKING ) &&
         ( uoflags & USTRUCT_TILE::FLAG_DOOR ) )
    {
      // example: 0xf5 secret door
      mapflags |= FLAG::BLOCKSIGHT;
    }
    else if ( ( uoflags & USTRUCT_TILE::FLAG_WALL ) && ( ~uoflags & USTRUCT_TILE::FLAG_BLOCKING ) )
    {
      // example: 0x245 dungeon arch
    }
    else if ( uoflags & ( USTRUCT_TILE::FLAG_BLOCKING | USTRUCT_TILE::FLAG_PLATFORM |
                          USTRUCT_TILE::FLAG_HALF_HEIGHT ) )
    {
      mapflags |= FLAG::BLOCKSIGHT;
    }
  }

  if ( Clib::flags_set( uoflags, USTRUCT_TILE::FLAG_FLOOR | USTRUCT_TILE::FLAG_LIQUID |
                                     USTRUCT_TILE::FLAG_BLOCKING ) )
  {
    // not blocking
  }
  else if ( ( uoflags & USTRUCT_TILE::FLAG_BLOCKING ) )
  {
    mapflags |= FLAG::BLOCKING;
  }
  if ( uoflags & USTRUCT_TILE::FLAG_ROOF )
  {
    mapflags |= FLAG::BLOCKING;
  }
  else if ( Clib::flags_clear( uoflags, USTRUCT_TILE::FLAG_WALL ) &&
            Clib::flags_clear( uoflags, USTRUCT_TILE::FLAG_BLOCKING ) )
  {
    mapflags |= FLAG::MOVABLE;
  }

  // the following flags are probably not useful:

  // everything allows overflight above it.
  // mapflags |= FLAG::OVERFLIGHT;

  if ( cfg_show_roof_and_platform_warning )
    if ( ( mapflags & FLAG::BLOCKING ) && ( mapflags & ( FLAG::MOVELAND | FLAG::MOVESEA ) ) )
      INFO_PRINTLN( "Warning: Tile {:#x} uses Roof- and Platform-Flag at same time.", tile );

  return mapflags;
}

u32 polflags_from_landtileflags( unsigned short tile, u32 lt_flags )
{
  if ( ~lt_flags & USTRUCT_TILE::FLAG_BLOCKING )
  {  // this seems to be the default.
    // used to set FLOOR here only.
    lt_flags |= USTRUCT_TILE::FLAG_PLATFORM;
  }
  lt_flags |= USTRUCT_TILE::FLAG_FLOOR;
  lt_flags |= USTRUCT_TILE::FLAG_HALF_HEIGHT;  // the entire map is this way

  unsigned int flags = polflags_from_tileflags(
      tile, lt_flags, false, false );  // Land tiles shouldn't worry about noshoot or windows
  return flags;
}

unsigned int readflags( Clib::ConfigElem& elem )
{
  bool flag_moveland = elem.remove_bool( "MoveLand", false );
  bool flag_movesea = elem.remove_bool( "MoveSea", false );
  bool flag_blocksight = elem.remove_bool( "BlockSight", false );
  bool flag_overflight = elem.remove_bool( "OverFlight", true );
  bool flag_allowdropon = elem.remove_bool( "AllowDropOn", false );
  bool flag_gradual = elem.remove_bool( "Gradual", false );
  bool flag_stackable = elem.remove_bool( "Stackable", false );
  bool flag_blocking = elem.remove_bool( "Blocking", false );
  bool flag_movable = elem.remove_bool( "Movable", false );
  bool flag_equippable = elem.remove_bool( "Equippable", false );
  bool flag_prepend_a = elem.remove_bool( "DescPrependA", false );
  bool flag_prepend_an = elem.remove_bool( "DescPrependAn", false );
  unsigned int flags = 0;

  if ( flag_moveland )
    flags |= FLAG::MOVELAND;
  if ( flag_movesea )
    flags |= FLAG::MOVESEA;
  if ( flag_blocksight )
    flags |= FLAG::BLOCKSIGHT;
  if ( flag_overflight )
    flags |= FLAG::OVERFLIGHT;
  if ( flag_allowdropon )
    flags |= FLAG::ALLOWDROPON;
  if ( flag_gradual )
    flags |= FLAG::GRADUAL;
  if ( flag_stackable )
    flags |= FLAG::STACKABLE;
  if ( flag_blocking )
    flags |= FLAG::BLOCKING;
  if ( flag_movable )
    flags |= FLAG::MOVABLE;
  if ( flag_equippable )
    flags |= FLAG::EQUIPPABLE;
  if ( flag_prepend_a )
    flags |= FLAG::DESC_PREPEND_A;
  if ( flag_prepend_an )
    flags |= FLAG::DESC_PREPEND_AN;

  return flags;
}
}  // namespace Plib
}  // namespace Pol
