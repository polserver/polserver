/*
History
=======
2009/09/03 MuadDib:   Relocation of multi related cpp/h
2009/12/02 Turley:    added config.max_tile_id - Tomi

Notes
=======

*/

#include "customhouses.h"
#include "../polcfg.h"

int BlockIDs[] = {  0x3EE, 0x709, 0x71E, 0x721,
				    0x738, 0x750, 0x76C, 0x788,
				    0x7A3, 0x7BA };

int StairSeqs[] = { 0x3EF, 0x70A, 0x722, 0x739,
				    0x751, 0x76D, 0x789, 0x7A4 };

int StairIDs[] = {  0x71F, 0x736, 0x737, 0x749,
				    0x7BB, 0x7BC };

bool CustomHouseDesign::IsStairBlock( u16 id )
{
	id &= config.max_tile_id;
	int delta = -1;

	for ( unsigned int i = 0; delta < 0 && (i <  (sizeof BlockIDs) / (sizeof( int))); ++i )
		delta = ( BlockIDs[i] - id );

	return ( delta == 0 );
}

bool CustomHouseDesign::IsStair( u16 id, int& dir )
{
	id &= config.max_tile_id;
	int delta = -4;

	for ( unsigned int i = 0; delta < -3 && (i < (sizeof StairSeqs) / (sizeof (int))); ++i )
		delta = ( StairSeqs[i] - id );

	if ( delta >= -3 && delta <= 0 )
	{
		dir = -delta;
		return true;
	}

	delta = -1;

	for ( unsigned int i = 0; delta < 0 && (i < (sizeof StairIDs) / (sizeof (int))); ++i )
	{
		delta = ( StairIDs[i] - id );
		dir = i % 4;
	}

	return ( delta == 0 );
}

bool CustomHouseDesign::DeleteStairs( u16 id, s32 x, s32 y, s8 z )
{
    int floor_num = z_to_custom_house_table(z);

    int xidx = x + xoff;
    int yidx = y + yoff;
    if(!ValidLocation(xidx,yidx))
        return false;

	if ( IsStairBlock( id ) )
	{
        HouseFloorZColumn::iterator itr;
        int xidx = x + xoff;
        int yidx = y + yoff;
        for( itr = Elements[floor_num].data.at(xidx).at(yidx).begin(); 
             itr !=Elements[floor_num].data.at(xidx).at(yidx).end(); 
             ++itr)
        {
            if( itr->z == (z+5))
            {
                id = itr->graphic;
                z = itr->z;
                if(!IsStairBlock(id))
                    break;
            }
        }
	}

	int dir = 0;

	if ( !IsStair( id, dir ) )
		return false;

	int height = ((z - 7) % 20) / 5;

	int xStart, yStart;
	int xInc, yInc;

	switch ( dir )
	{
		default:
		case 0: // North
		{
			xStart = x;
			yStart = y + height;
			xInc = 0;
			yInc = -1;
			break;
		}
		case 1: // West
		{
			xStart = x + height;
			yStart = y;
			xInc = -1;
			yInc = 0;
			break;
		}
		case 2: // South
		{
			xStart = x;
			yStart = y - height;
			xInc = 0;
			yInc = 1;
			break;
		}
		case 3: // East
		{
			xStart = x - height;
			yStart = y;
			xInc = 1;
			yInc = 0;
			break;
		}
	}

	int zStart = z - (height * 5);

	for ( int i = 0; i < 4; ++i )
	{
		x = xStart + (i * xInc);
		y = yStart + (i * yInc);

		for ( int j = 0; j <= i; ++j )
            Erase(x,y,static_cast<u8>(zStart + (j * 5)), 2);

        ReplaceDirtFloor(x,y);
	}

	return true;
}


HouseFloorZColumn* CustomHouseElements::GetElementsAt(s32 xoffset, s32 yoffset)
{
    u32 x = xoffset+xoff;
    u32 y = yoffset+yoff;
    return &(data.at(x).at(y));
}
void CustomHouseElements::AddElement( CUSTOM_HOUSE_ELEMENT& elem )
{
    u32 x = elem.xoffset + xoff;
    u32 y = elem.yoffset + yoff;

    data.at(x).at(y).push_back(elem);
}



