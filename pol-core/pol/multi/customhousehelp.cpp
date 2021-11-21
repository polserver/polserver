/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include "clib/rawtypes.h"
#include "plib/systemstate.h"

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

bool CustomHouseDesign::DeleteStairs( u16 id, Core::Pos3d pos )
{
  int floor_num = z_to_custom_house_table( pos.z() );
  if ( floor_num == -1 )
    return false;

  Core::Pos3d idx = pos + _offset;
  if ( !ValidLocation( idx.xy() ) )
    return false;

  if ( IsStairBlock( id ) )
  {
    for ( const auto& ele : Elements[floor_num].GetFloor()[idx.x()][idx.y()] )
    {
      if ( ele.z == ( pos.z() + 5 ) )
      {
        id = ele.graphic;
        pos.z( ele.z );
        if ( !IsStairBlock( id ) )
          break;
      }
    }
  }

  int dir = 0;

  if ( !IsStair( id, dir ) )
    return false;

  s8 erase_height = ( ( pos.z() - 7 ) % 20 ) / 5;

  pos -= Core::Vec3d( 0, 0, erase_height * 5 );
  s8 xInc, yInc;

  switch ( dir )
  {
  default:
  case 0:  // North
  {
    pos += Core::Vec2d( 0, erase_height );
    xInc = 0;
    yInc = -1;
    break;
  }
  case 1:  // West
  {
    pos += Core::Vec2d( erase_height, 0 );
    xInc = -1;
    yInc = 0;
    break;
  }
  case 2:  // South
  {
    pos -= Core::Vec2d( 0, erase_height );
    xInc = 0;
    yInc = 1;
    break;
  }
  case 3:  // East
  {
    pos -= Core::Vec2d( erase_height, 0 );
    xInc = 1;
    yInc = 0;
    break;
  }
  }

  for ( s8 i = 0; i < 4; ++i )
  {
    Core::Pos3d v = pos + Core::Vec2d( i * xInc, i * yInc );

    for ( s8 j = 0; j <= i; ++j )
      Erase( v + Core::Vec3d( 0, 0, j * 5 ), 2 );

    ReplaceDirtFloor( v.xy() );
  }

  return true;
}

CustomHouseElements::CustomHouseElements() : data(), size(), offset() {}

void CustomHouseElements::SetOffset( Core::Vec2d off )
{
  offset = std::move( off );
}
void CustomHouseElements::SetSize( Core::Pos2d dim )
{
  size = std::move( dim );
  data.resize( size.x() );
  for ( size_t x = 0; x < size.x(); ++x )
    data.at( x ).resize( size.y() );
}

size_t CustomHouseElements::estimatedSize() const
{
  size_t csize = sizeof( CustomHouseElements );
  csize += 3 * sizeof( HouseFloor* ) + data.capacity() * sizeof( HouseFloor );
  for ( const auto& floor : data )
  {
    csize += 3 * sizeof( std::list<CUSTOM_HOUSE_ELEMENT>* ) +
             floor.capacity() * sizeof( std::list<CUSTOM_HOUSE_ELEMENT> );
    for ( const auto& l : floor )
    {
      csize += 3 * sizeof( CUSTOM_HOUSE_ELEMENT* ) + l.size() * sizeof( CUSTOM_HOUSE_ELEMENT );
    }
  }
  return csize;
}

HouseFloor& CustomHouseElements::GetFloor()
{
  return data;
}
const HouseFloor& CustomHouseElements::GetFloor() const
{
  return data;
}

HouseFloorZColumn* CustomHouseElements::GetElementsAt( const Core::Vec2d& off )
{
  Core::Pos2d p = Core::Pos2d( 0, 0 ) + off + offset;
  return &( data.at( p.x() ).at( p.y() ) );
}
HouseFloorZColumn* CustomHouseElements::GetElementsAt( const Core::Pos2d& p )
{
  return &( data.at( p.x() ).at( p.y() ) );
}
void CustomHouseElements::AddElement( CUSTOM_HOUSE_ELEMENT& elem )
{
  Core::Pos2d p = Core::Pos2d( 0, 0 ) + elem.offset + offset;
  data.at( p.x() ).at( p.y() ).push_back( elem );
}
}  // namespace Multi
}  // namespace Pol
