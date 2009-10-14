/*
History
=======


Notes
=======

*/

#ifndef __UFACING_H
#define __UFACING_H

#include "../poltype.h"
#include "../uobject.h"
#include "charactr.h"

static UFACING GetRandomFacing()
{
	int rval = random_int( 8 );
	return static_cast<UFACING>( rval );
}

const char* FacingStr( UFACING facing )
{
	switch( facing )
	{
	case FACING_N: return "N";
	case FACING_S: return "S";
	case FACING_E: return "E";
	case FACING_W: return "W";
	case FACING_NE: return "NE";
	case FACING_NW: return "NW";
	case FACING_SE: return "SE";
	case FACING_SW: return "SW";
	}
	return "";
}

int adjustments[] =
{
	0,
	+1,
	-1,
	+2,
	-2,
	+3,
	-3
};
const int N_ADJUST = arsize( adjustments );

UFACING direction_toward( const Character* src, const UObject* idst )
{
	const UObject* dst = idst->toplevel_owner();
	if (src->x < dst->x)        // East to target
	{
		if (src->y < dst->y)
			return FACING_SE;
		else if (src->y == dst->y)
			return FACING_E;
		else /* src->y > dst->y */
			return FACING_NE;
	}
	else if (src->x == dst->x)
	{
		if (src->y < dst->y)
			return FACING_S;
		else if (src->y > dst->y)
			return FACING_N;
	}
	else /* src->x > dst->x */  // West to target
	{
		if (src->y < dst->y)
			return FACING_SW;
		else if (src->y == dst->y)
			return FACING_W;
		else /* src->y > dst->y */
			return FACING_NW;
	}
	return FACING_N;
}

UFACING direction_toward( const Character* src, xcoord to_x, ycoord to_y )
{

	if (src->x < to_x)        // East to target
	{
		if (src->y < to_y)
			return FACING_SE;
		else if (src->y == to_y)
			return FACING_E;
		else /* src->y > dst->y */
			return FACING_NE;
	}
	else if (src->x == to_x)
	{
		if (src->y < to_y)
			return FACING_S;
		else if (src->y > to_y)
			return FACING_N;
	}
	else /* src->x > dst->x */  // West to target
	{
		if (src->y < to_y)
			return FACING_SW;
		else if (src->y == to_y)
			return FACING_W;
		else /* src->y > dst->y */
			return FACING_NW;
	}
	return FACING_N;
}

UFACING away_cvt[8] = {
	FACING_S,
	FACING_SW,
	FACING_W,
	FACING_NW,
	FACING_N,
	FACING_NE,
	FACING_E,
	FACING_SE
};

UFACING direction_away( const Character* src, const UObject* idst )
{
	UFACING toward = direction_toward(src,idst);
	UFACING away = away_cvt[ static_cast<int>(toward) ];
	return away;
}

UFACING direction_away( const Character* src, xcoord from_x, ycoord from_y )
{
	UFACING toward = direction_toward(src,from_x, from_y);
	UFACING away = away_cvt[ static_cast<int>(toward) ];
	return away;
}

#endif // UFACING_H