/*
History
=======


Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "boundbox.h"

bool BoundingBox::contains( unsigned short x, unsigned short y ) const
{
	for( Areas::const_iterator itr = areas.begin(); itr != areas.end(); ++itr )
	{
		if (x >= itr->topleft.x && x <= itr->bottomright.x &&
			y >= itr->topleft.y && y <= itr->bottomright.y)
		{
			return true;
		}
	}
	return false;
}

void BoundingBox::addarea( const Area& area )
{
	areas.push_back( area );
}


