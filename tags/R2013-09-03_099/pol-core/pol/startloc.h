/*
History
=======


Notes
=======

*/

#ifndef STARTLOC_H
#define STARTLOC_H

#include <string>
#include <vector>

class Coordinate
{
public:
	unsigned short x;
	unsigned short y;
	char z;

	Coordinate( unsigned short i_x, unsigned short i_y, char i_z ) :
		x(i_x), y(i_y), z(i_z) {}
    /*Coordinate( const Coordinate& );
    const Coordinate& operator=( const Coordinate& );*/
};

class StartingLocation
{
public:
    std::string city;
    std::string desc;
	unsigned short mapid;
	unsigned long cliloc_desc;
	/* NOTE: this list will be selected from randomly.  
	   By placing only one coordinate in a StartingLocation,
	   the system will behave as standard UO - ie, you pick
	   the exact point you come into the world.
	*/
    std::vector<Coordinate> coords;

    Coordinate select_coordinate() const;
};

#endif
