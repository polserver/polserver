/*
History
=======


Notes
=======

*/

#ifndef __BOUNDBOX_H
#define __BOUNDBOX_H

#include "../../bscript/execmodl.h"

struct Point
{
	unsigned short x;
	unsigned short y;
};

struct Area
{
	Point topleft;
	Point bottomright;
};

class BoundingBox
{
public:
	bool contains( unsigned short x, unsigned short y ) const;
	bool read( std::string areadesc );
	void addarea( const Area& area );
private:
	typedef std::vector<Area> Areas;
	Areas areas;
};

#endif // BOUNDBOX_H

