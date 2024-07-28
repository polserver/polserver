/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include "../../clib/rawtypes.h"
#include "../../clib/stlutil.h"
#include "../../plib/systemstate.h"
#include "customhouses.h"

namespace Pol
{
namespace Multi
{
int BlockIDs[] = {
    0x3EE,   // Sandstone
    0x709,   // Tan Marble
    0x71E,   // Granite
    0x721,   // Light Wood
    0x738,   // Dark Wood
    0x750,   // Light Stone
    0x76C,   // Sandstone Brick
    0x788,   // Weathered Stone
    0x7A3,   // Grey Stone
    0x7BA,   // Red
    0x35D2,  // Crystal
    0x3609,  // Shadow
    0x4317,  // Gargish 1 & 2
    0x4318   // Gargish 3
};

int StairSeqs[] = {
    0x3EF,  // Sandstone
    0x70A,  // Tan Marble
    0x722,  // Light Wood
    0x739,  // Dark Wood
    0x751,  // Light Stone
    0x76D,  // Sandstone Brick
    0x789,  // Weathered Stone
    0x7A4   // Grey Stone
};
//                      N,      W,      S,      E
int StairIDs[] = {
    0x71F,  0x736,  0x737,
    0x749,          // Granite
    0x7BB,  0x7BC,  // Red
    0x35D4, 0x35D3, 0x35D6,
    0x35D5,  // Crystal
    0x360B, 0x360A, 0x360D,
    0x360C,  // Shadow
    0x4360, 0x435E, 0x435F,
    0x4361,  // Gargish 1
    0x435C, 0x435A, 0x435B,
    0x435C,  // Gargish 2
    0x4364, 0x4362, 0x4363,
    0x4365  // Gargish 3
};

bool CustomHouseDesign::IsStairBlock( u16 id )
{
  id &= Plib::systemstate.config.max_tile_id;
  int delta = -1;

  for ( unsigned int i = 0; delta < 0 && ( i < ( sizeof BlockIDs ) / ( sizeof( int ) ) ); ++i )
    delta = ( BlockIDs[i] - id );

  return ( delta == 0 );
}

bool CustomHouseDesign::IsStair( u16 id, int& dir )
{
  id &= Plib::systemstate.config.max_tile_id;
  int delta = -4;

  for ( unsigned int i = 0; delta < -3 && ( i < ( sizeof StairSeqs ) / ( sizeof( int ) ) ); ++i )
    delta = ( StairSeqs[i] - id );

  if ( delta >= -3 && delta <= 0 )
  {
    dir = -delta;
    return true;
  }

  delta = -1;

  for ( unsigned int i = 0; delta < 0 && ( i < ( sizeof StairIDs ) / ( sizeof( int ) ) ); ++i )
  {
    delta = ( StairIDs[i] - id );
    dir = i % 4;
  }

  return ( delta == 0 );
}

bool CustomHouseDesign::DeleteStairs( u16 id, s32 x, s32 y, s8 z )
{
  int floor_num = z_to_custom_house_table( z );
  if ( floor_num == -1 )
    return false;

  u32 xidx = x + xoff;
  u32 yidx = y + yoff;
  if ( !ValidLocation( xidx, yidx ) )
    return false;

  if ( IsStairBlock( id ) )
  {
    for ( HouseFloorZColumn::iterator itr = Elements[floor_num].data.at( xidx ).at( yidx ).begin(),
                                      itrend = Elements[floor_num].data.at( xidx ).at( yidx ).end();
          itr != itrend; ++itr )
    {
      if ( itr->z == ( z + 5 ) )
      {
        id = itr->graphic;
        z = itr->z;
        if ( !IsStairBlock( id ) )
          break;
      }
    }
  }

  int dir = 0;

  if ( !IsStair( id, dir ) )
    return false;

  int erase_height = ( ( z - 7 ) % 20 ) / 5;

  int xStart, yStart;
  int xInc, yInc;

  switch ( dir )
  {
  default:
  case 0:  // North
  {
    xStart = x;
    yStart = y + erase_height;
    xInc = 0;
    yInc = -1;
    break;
  }
  case 1:  // West
  {
    xStart = x + erase_height;
    yStart = y;
    xInc = -1;
    yInc = 0;
    break;
  }
  case 2:  // South
  {
    xStart = x;
    yStart = y - erase_height;
    xInc = 0;
    yInc = 1;
    break;
  }
  case 3:  // East
  {
    xStart = x - erase_height;
    yStart = y;
    xInc = 1;
    yInc = 0;
    break;
  }
  }

  int zStart = z - ( erase_height * 5 );

  for ( int i = 0; i < 4; ++i )
  {
    x = xStart + ( i * xInc );
    y = yStart + ( i * yInc );

    for ( int j = 0; j <= i; ++j )
      Erase( x, y, static_cast<u8>( zStart + ( j * 5 ) ), 2 );

    ReplaceDirtFloor( x, y );
  }

  return true;
}

CustomHouseElements::CustomHouseElements() : data(), height( 0 ), width( 0 ), xoff( 0 ), yoff( 0 )
{
}

CustomHouseElements::CustomHouseElements( u32 _height, u32 _width, s32 xoffset, s32 yoffset )
    : height( _height ), width( _width ), xoff( xoffset ), yoff( yoffset )
{
  SetWidth( _width );
  SetHeight( _height );
}
CustomHouseElements::~CustomHouseElements() {}

void CustomHouseElements::SetHeight( u32 _height )
{
  height = _height;
  for ( size_t i = 0; i < width; i++ )
    data.at( i ).resize( height );
}

void CustomHouseElements::SetWidth( u32 _width )
{
  width = _width;
  data.resize( width );
}

size_t CustomHouseElements::estimatedSize() const
{
  size_t size = sizeof( CustomHouseElements ) + Clib::memsize( data );
  for ( const auto& floor : data )
  {
    size += Clib::memsize( floor );
    for ( const auto& l : floor )
    {
      size += Clib::memsize( l );
    }
  }
  return size;
}

HouseFloorZColumn* CustomHouseElements::GetElementsAt( s32 xoffset, s32 yoffset )
{
  u32 x = xoffset + xoff;
  u32 y = yoffset + yoff;
  return &( data.at( x ).at( y ) );
}
void CustomHouseElements::AddElement( CUSTOM_HOUSE_ELEMENT& elem )
{
  u32 x = elem.xoffset + xoff;
  u32 y = elem.yoffset + yoff;

  data.at( x ).at( y ).push_back( elem );
}
}  // namespace Multi
}  // namespace Pol
