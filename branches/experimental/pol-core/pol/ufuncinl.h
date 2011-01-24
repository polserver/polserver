/*
History
=======


Notes
=======

*/

#ifndef UFUNCINL_H
#define UFUNCINL_H

#include <stdlib.h>

inline bool inrangex_inline( const Character *c1, const Character *c2, int maxdist )
{
	return ( (abs( c1->x - c2->x ) <= maxdist) &&
		     (abs( c1->y - c2->y ) <= maxdist) );
}

inline bool inrangex_inline( const UObject *c1, unsigned short x, unsigned short y, int maxdist )
{
	return ( (abs( c1->x - x ) <= maxdist) &&
		     (abs( c1->y - y ) <= maxdist) );
}

#endif
