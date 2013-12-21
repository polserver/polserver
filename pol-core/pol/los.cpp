/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif


#include <stdlib.h>

#include "../clib/strutil.h"

#include "item/item.h"
#include "polcfg.h"
#include "uobject.h"
#include "uvars.h"
#include "ustruct.h"
#include "uofile.h"
#include "udatfile.h"
#include "uworld.h"
#include "los.h"

#define USE_2D_LOS 0
#define USE_3D_LOS 1
namespace Pol {
  namespace Core {
	const int los_range = 20;
	const int z_los_range = 60; // unused as yet

	/*
		To determine LOS, a 3D line is drawn from the top of the attacker
		to the top of the target.

		An 3D extension of Bresenham's algorithm is used to draw the line.

		Each point along the way is checked for items and statics of any kind.
		Only integer addition and subtraction is required.

		The line always starts at the point with the lowest Y-coordinate,
		or if the Y coordinates are the same, the point with the lowest
		Z-coordinate.  This way, if A -> B has LOS, then B -> A has LOS,
		always.  (Bresenham lines can occupy different points, given
		the same endpoints, depending on which point is used as the start)

		Zero-height items (floor tiles, mostly) are treated as 1-height
		items, sitting at 1 lower Z coordinate.  This makes floors solid.
		(A weirdness: On a floor at height 72, characters walk around at Z=73,
		but items are placed at Z=72.  This is somewhat unfortunate.)

		Ghosts standing in the same place as a door should be handled
		correctly, because the start and end points are checked for
		obstacles.  I'm not sure how windows will fit in the picture.
		Indeed, there are probably flags in statics that I should pay
		attention to (floor, blocking, window, stairway) but I don't yet.

		Note, all obstacles found to obstruct the LOS are checked
		to make sure they are neither the attacker nor the target,
		since both endpoints are checked.  This is actually unnecessary
		when both attacker and defender are characters, which ends up
		being all the time when it matters.

		Possible optimizations/improvements:
		Read statics file once per X/Y change, rather than each X/Y/Z change
		*/

	/**************************** 3D LOS CHECKING *******************************/

#if USE_3D_LOS


	bool dynamic_item_blocks_los( const LosObj& att, const LosObj& target,
								  int x, int y, int z, Realm* realm )
	{
	  unsigned short wx, wy;
	  w_convert( x, y, wx, wy );

	  ZoneItems& witems = realm->zone[wx][wy].items;

	  for ( ZoneItems::iterator itr = witems.begin(), end = witems.end(); itr != end; ++itr )
	  {
		Item* item = *itr;
		if ( ( item->x == x ) &&
			 ( item->y == y ) &&
			 ( tile_flags( item->graphic ) & USTRUCT_TILE::FLAG_WALKBLOCK ) )
		{
		  if ( z >= item->z && z < item->z + item->height )
		  {
			// NOTE, we don't break with an early 'false' indicator
			// if the serials DO match, because ghosts can be in the
			// same place as a door, and the line can start at either
			// the attacker or the target.

			if ( item->serial != target.serial && item->serial != att.serial )
			{
#if ENABLE_POLTEST_OUTPUT
			  cout << "LOS blocked by " << item->description() << endl;
#endif
			  return true;
			}
		  }
		}
	  }
	  return false;
	}

	bool static_item_blocks_los( int x, int y, int z )
	{
	  typedef vector<StaticRec> StaticRecVector;
	  static StaticRecVector vec;
	  vec.clear();
	  readstatics( vec, x, y );
	  readmultis( vec, x, y );
	  for ( StaticRecVector::const_iterator itr = vec.begin();
			itr != vec.end();
			++itr )
	  {
		int ob_ht = tileheight( itr->graphic );
		int ob_z = itr->z;
#if ENABLE_POLTEST_OUTPUT
		cout << "static type " << hexint( itr->graphic )
		  << " (flags " << hexint( tile_flags( itr->graphic ) ) << ", ht=" << ob_ht << ")"
		  << " at z-coord " << (int)itr->z << endl;
#endif

		if ( ob_ht == 0 )
		{
		  --ob_z;
		  ++ob_ht;
		}



		if ( z >= ob_z && z < ob_z + ob_ht )
		{
#if ENABLE_POLTEST_OUTPUT
		  cout << "LOS blocked by static object" << endl;
#endif
		  return true;
		}
	  }
	  return false;
	}

	// Is any object occupying point (x,y,z) that is neither attacker nor target?
	bool los_blocked( const LosObj& att, const LosObj& target,
					  int x, int y, int z )
	{
	  //cout << "los_blocked: (" << x << "," << y << "," << z << ")" << endl;
	  // if the target inhabits the location, LOS can't be blocked:
	  if ( att.x == x &&
		   att.y == y &&
		   att.z <= z &&
		   z < att.z + att.obj_height )
	  {
		return false;
	  }
	  if ( target.x == x &&
		   target.y == y &&
		   target.z <= z &&
		   z < target.z + target.obj_height )
	  {
		return false;
	  }

	  if ( config.exp_los_checks_map )
	  {
		USTRUCT_MAPINFO mi;
		rawmapinfo( x, y, &mi );
		if ( ( landtile_flags[mi.landtile] &
		  ( USTRUCT_TILE::FLAG_BLOCKING | USTRUCT_TILE::FLAG_WALL ) )
		  == ( USTRUCT_TILE::FLAG_BLOCKING | USTRUCT_TILE::FLAG_WALL ) )
		  return true;
	  }

	  return dynamic_item_blocks_los( att, target, x, y, z ) ||
		static_item_blocks_los( x, y, z );
	}

	// absolute value of a 
#define ABS(a) (((a)<0) ? -(a) : (a))

	// take sign of a, either -1, 0, or 1 
#define ZSGN(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)

	bool uo_has_los( const LosObj& att, const LosObj& tgt )
	{
	  int x1, y1, z1; // one of the endpoints
	  int x2, y2, z2; // the other endpoint
	  int xd, yd, zd;
	  int x, y, z;
	  int ax, ay, az;
	  int sx, sy, sz;
	  int dx, dy, dz;

	  if ( ( att.y < tgt.y ) ||
		   ( att.y == tgt.y && att.z < tgt.z ) )
	  {
		x1 = att.x;  y1 = att.y;  z1 = att.z + att.look_height;
		x2 = tgt.x;  y2 = tgt.y;  z2 = tgt.z + tgt.look_height;
	  }
	  else
	  {
		x1 = tgt.x;  y1 = tgt.y;  z1 = tgt.z + tgt.look_height;
		x2 = att.x;  y2 = att.y;  z2 = att.z + att.look_height;
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


#endif

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

  }
}
