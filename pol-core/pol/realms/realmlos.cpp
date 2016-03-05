/** @file
 *
 * @par History
 */

#include "realm.h"
#include "../../plib/mapcell.h"
#include "../../plib/mapserver.h"
#include "../../plib/inmemorymapserver.h"
#include "../../plib/mapshape.h"

#include "../uworld.h" // TODO move 'world' into Realm
#include "../item/item.h"
#include "../los.h"
#include "../clidata.h"

namespace Pol
{
namespace Realms
{
const int los_range = 20;
//const int z_los_range = 60; // unused as yet

/**
 * @defgroup los3d 3D LOS CHECKING
 *
 * To determine LOS, a 3D line is drawn from the top of the attacker
 * to the top of the target.
 *
 * An 3D extension of Bresenham's algorithm is used to draw the line.
 *
 * Each point along the way is checked for items and statics of any kind.
 * Only integer addition and subtraction is required.
 *
 * The line always starts at the point with the lowest Y-coordinate,
 * or if the Y coordinates are the same, the point with the lowest
 * Z-coordinate.  This way, if A -> B has LOS, then B -> A has LOS,
 * always.  (Bresenham lines can occupy different points, given
 * the same endpoints, depending on which point is used as the start)
 *
 * Zero-height items (floor tiles, mostly) are treated as 1-height
 * items, sitting at 1 lower Z coordinate.  This makes floors solid.
 * (A weirdness: On a floor at height 72, characters walk around at Z=73,
 * but items are placed at Z=72.  This is somewhat unfortunate.)
 *
 * @note All obstacles found to obstruct the LOS are checked
 * to make sure they are neither the attacker nor the target,
 * since both endpoints are checked.  This is actually unnecessary
 * when both attacker and defender are characters, which ends up
 * being all the time when it matters.
 *
 * @par Possible optimizations/improvements
 * Read statics file once per X/Y change, rather than each X/Y/Z change
 */

/**
 * @ingroup los3d
 */
bool Realm::dynamic_item_blocks_los( const Core::LosObj& att, const Core::LosObj& target,
                                     unsigned short x, unsigned short y, short z ) const
{
  unsigned short wx, wy;
  Core::zone_convert_clip( x, y, this, &wx, &wy );
  Core::ZoneItems& witems = zone[wx][wy].items;

  for ( const auto& item : witems )
  {
    if ( ( item->x == x ) &&
         ( item->y == y ) )
    {
      u32 flags = Core::tile_flags( item->graphic );

      if ( flags & Plib::FLAG::BLOCKSIGHT )
      {
        if ( item->z <= z &&
             z < item->z + item->height )
        {
          if ( item->serial != target.serial && item->serial != att.serial )
          {
#if ENABLE_POLTEST_OUTPUT
            INFO_PRINT << "LOS blocked by " << item->description( ) << "\n";
#endif
            return true;
          }
        }
      }
    }
  }
  return false;
}

/**
 * @ingroup los3d
 */
bool Realm::static_item_blocks_los( unsigned short x, unsigned short y, short z ) const
{
  static Plib::MapShapeList shapes;
  shapes.clear();

  getmapshapes( shapes, x, y, Plib::FLAG::BLOCKSIGHT );
  readmultis( shapes, x, y, Plib::FLAG::BLOCKSIGHT );
  for ( Plib::MapShapeList::const_iterator itr = shapes.begin(), end = shapes.end(); itr != end; ++itr )
  {
    const Plib::MapShape& shape = ( *itr );
    short ob_ht = shape.height;
    short ob_z = shape.z;
#if ENABLE_POLTEST_OUTPUT
    INFO_PRINT << "static type 0x" << fmt::hexu(itr->graphic)
               << " (flags 0x" << fmt::hexu( tile_flags( itr->graphic ) ) << ", ht=" << ob_ht << ")"
               << " at z-coord " << (int)itr->z << "\n";
#endif

    if ( ob_ht == 0 ) // treat a 0-height object as a 1-height object at position z-1
    {
      --ob_z;
      ++ob_ht;
    }



    if ( ob_z <= z &&
         z < ob_z + ob_ht )
    {
#if ENABLE_POLTEST_OUTPUT
      INFO_PRINT << "LOS blocked by static object\n";
#endif
      return true;
    }
  }
  return false;
}

/**
 * Is any object occupying point (x,y,z) that is neither attacker nor target?
 *
 * @ingroup los3d
 */
bool Realm::los_blocked( const Core::LosObj& att, const Core::LosObj& target,
                         unsigned short x, unsigned short y, short z ) const
{
  // if the target inhabits the location, LOS can't be blocked:
  if ( att.x == x &&
       att.y == y &&
       att.z <= z &&
       z <= att.z + att.obj_height ) // LE to allow for 0-height target
  {
    return false;
  }
  if ( target.x == x &&
       target.y == y &&
       target.z <= z &&
       z <= target.z + target.obj_height ) // LE to allow for 0-height target
  {
    return false;
  }

  return dynamic_item_blocks_los( att, target, x, y, z ) ||
         static_item_blocks_los( x, y, z );
}

/// absolute value of a
#define ABS(a) (((a)<0) ? -(a) : (a))

/// take sign of a, either -1, 0, or 1
#define ZSGN(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)

/**
* @ingroup los3d
*/
bool Realm::has_los( const Core::LosObj& att, const Core::LosObj& tgt ) const
{
  short x1, y1, z1; // one of the endpoints
  short x2, y2, z2; // the other endpoint
  short xd, yd, zd;
  short x, y, z;
  short ax, ay, az;
  short sx, sy, sz;
  short dx, dy, dz;

  if ( ( att.y < tgt.y ) ||
       ( att.y == tgt.y && att.z < tgt.z ) )
  {
    x1 = att.x;
    y1 = att.y;
    z1 = att.z + att.look_height;
    x2 = tgt.x;
    y2 = tgt.y;
    z2 = tgt.z + tgt.look_height;
  }
  else
  {
    x1 = tgt.x;
    y1 = tgt.y;
    z1 = tgt.z + tgt.look_height;
    x2 = att.x;
    y2 = att.y;
    z2 = att.z + att.look_height;
  }

  dx = x2 - x1;
  dy = y2 - y1;
  dz = z2 - z1;

  if ( abs( dx ) > los_range || abs( dy ) > los_range )
    return false;

  if ( !dx && !dy )
  {
    if ( !dz )
      return true;
    if ( att.z <= tgt.z &&
         tgt.z <= att.z + att.obj_height )
    {
      return true;
    }
    if ( att.z <= tgt.z + tgt.look_height &&
         tgt.z + tgt.look_height <= att.z + att.obj_height )
    {
      return true;
    }
  }

  ax = ABS( dx ) << 1;
  ay = ABS( dy ) << 1;
  az = ABS( dz ) << 1;

  sx = ZSGN( dx );
  sy = ZSGN( dy );
  sz = ZSGN( dz );

  x = x1;
  y = y1;
  z = z1;

  if ( ax >= ay && ax >= az )            // x dominant
  {
    yd = ay - ( ax >> 1 );
    zd = az - ( ax >> 1 );

    for ( ;; )
    {
      if ( los_blocked( att, tgt, x, y, z ) )
        return false;

      if ( x == x2 )
      {
        return true;
      }

      if ( yd >= 0 )
      {
        y += sy;
        yd -= ax;
      }

      if ( zd >= 0 )
      {
        z += sz;
        zd -= ax;
      }

      x += sx;
      yd += ay;
      zd += az;

    }
  }
  else if ( ay >= ax && ay >= az )            //y dominant
  {
    xd = ax - ( ay >> 1 );
    zd = az - ( ay >> 1 );

    for ( ;; )
    {
      if ( los_blocked( att, tgt, x, y, z ) )
        return false;

      if ( y == y2 )
      {
        return true;
      }

      if ( xd >= 0 )
      {
        x += sx;
        xd -= ay;
      }

      if ( zd >= 0 )
      {
        z += sz;
        zd -= ay;
      }

      y += sy;
      xd += ax;
      zd += az;

    }
  }
  else           // z dominant
  {
    xd = ax - ( az >> 1 );
    yd = ay - ( az >> 1 );

    for ( ;; )
    {
      if ( los_blocked( att, tgt, x, y, z ) )
        return false;

      if ( z == z2 )
      {
        return true;
      }

      if ( xd >= 0 )
      {
        x += sx;
        xd -= az;
      }

      if ( yd >= 0 )
      {
        y += sy;
        yd -= az;
      }

      z += sz;
      xd += ax;
      yd += ay;

    }
  }
  return true;
}
}
}


/****************************************************************************/

/* Original 3D bresenham algorithm from comp.unix.sources

//
// line3d was dervied from DigitalLine.c published as "Digital Line Drawing"
// by Paul Heckbert from "Graphics Gems", Academic Press, 1990
//
// 3D modifications by Bob Pendleton. The original source code was in the public
// domain, the author of the 3D version places his modifications in the
// public domain as well.
//
// line3d uses Bresenham's algorithm to generate the 3 dimensional points on a
// line from (x1, y1, z1) to (x2, y2, z2)
//
//

// find maximum of a and b
#define MAX(a,b) (((a)>(b))?(a):(b))

// absolute value of a
#define ABS(a) (((a)<0) ? -(a) : (a))

// take sign of a, either -1, 0, or 1
#define ZSGN(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)

point3d(x, y, z)
    int x, y, z;
{

    //output the point as you see fit

}

line3d(x1, y1, x2, y2, z1, z2)
    int x1, y1, x2, y2, z1, z2;
{
    int xd, yd, zd;
    int x, y, z;
    int ax, ay, az;
    int sx, sy, sz;
    int dx, dy, dz;

    dx = x2 - x1;
    dy = y2 - y1;
    dz = z2 - z1;

    ax = ABS(dx) << 1;
    ay = ABS(dy) << 1;
    az = ABS(dz) << 1;

    sx = ZSGN(dx);
    sy = ZSGN(dy);
    sz = ZSGN(dz);

    x = x1;
    y = y1;
    z = z1;

    if (ax >= MAX(ay, az))            // x dominant
    {
        yd = ay - (ax >> 1);
        zd = az - (ax >> 1);
        for (;;)
        {
            point3d(x, y, z);
            if (x == x2)
            {
                return;
            }

            if (yd >= 0)
            {
                y += sy;
                yd -= ax;
            }

            if (zd >= 0)
            {
                z += sz;
                zd -= ax;
            }

            x += sx;
            yd += ay;
            zd += az;
        }
    }
    else if (ay >= MAX(ax, az))            //y dominant
    {
        xd = ax - (ay >> 1);
        zd = az - (ay >> 1);
        for (;;)
        {
            point3d(x, y, z);
            if (y == y2)
            {
                return;
            }

            if (xd >= 0)
            {
                x += sx;
                xd -= ay;
            }

            if (zd >= 0)
            {
                z += sz;
                zd -= ay;
            }

            y += sy;
            xd += ax;
            zd += az;
        }
    }
    else if (az >= MAX(ax, ay))            // z dominant
    {
        xd = ax - (az >> 1);
        yd = ay - (az >> 1);
        for (;;)
        {
            point3d(x, y, z);
            if (z == z2)
            {
                return;
            }

            if (xd >= 0)
            {
                x += sx;
                xd -= az;
            }

            if (yd >= 0)
            {
                y += sy;
                yd -= az;
            }

            z += sz;
            xd += ax;
            yd += ay;
        }
    }
}

*/
