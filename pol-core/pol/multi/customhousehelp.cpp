/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include "../../clib/rawtypes.h"
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

bool CustomHouseDesign::DeleteStairs( u16 id, Core::Vec3d off )
{
  int floor_num = z_to_custom_house_table( off.z() );
  if ( floor_num == -1 )
    return false;

  Core::Pos2d idx = Core::Pos2d( 0, 0 ) + ( off + offset ).xy();
  if ( !ValidLocation( idx ) )
    return false;

  if ( IsStairBlock( id ) )
  {
    for ( HouseFloorZColumn::iterator
              itr = Elements[floor_num].data.at( idx.x() ).at( idx.y() ).begin(),
              itrend = Elements[floor_num].data.at( idx.x() ).at( idx.y() ).end();
          itr != itrend; ++itr )
    {
      if ( itr->z == ( off.z() + 5 ) )
      {
        id = itr->graphic;
        off.z( itr->z );
        if ( !IsStairBlock( id ) )
          break;
      }
    }
  }

  int dir = 0;

  if ( !IsStair( id, dir ) )
    return false;

  int erase_height = ( ( off.z() - 7 ) % 20 ) / 5;

  int xInc, yInc;

  switch ( dir )
  {
  default:
  case 0:  // North
  {
    off += Core::Vec2d( 0, erase_height );
    xInc = 0;
    yInc = -1;
    break;
  }
  case 1:  // West
  {
    off += Core::Vec2d( erase_height, 0 );
    xInc = -1;
    yInc = 0;
    break;
  }
  case 2:  // South
  {
    off -= Core::Vec2d( 0, erase_height );
    xInc = 0;
    yInc = 1;
    break;
  }
  case 3:  // East
  {
    off -= Core::Vec2d( erase_height, 0 );
    xInc = 1;
    yInc = 0;
    break;
  }
  }

  off -= Core::Vec3d( 0, 0, erase_height * 5 );

  for ( int i = 0; i < 4; ++i )
  {
    Core::Vec3d v = off + Core::Vec2d( i * xInc, i * yInc );

    for ( int j = 0; j <= i; ++j )
      Erase( v.xy(), static_cast<u8>( v.z() + j * 5 ), 2 );

    ReplaceDirtFloor( off.xy() );
  }

  return true;
}

CustomHouseElements::CustomHouseElements() : data(), size( 0, 0 ), offset( 0, 0 ) {}

CustomHouseElements::CustomHouseElements( Core::Pos2d width_height, Core::Vec2d off )
    : size( std::move( width_height ) ), offset( std::move( off ) )
{
  SetSize( size );
}

void CustomHouseElements::SetSize( const Core::Pos2d& width_height )
{
  size = width_height;
  data.resize( size.x() );
  for ( u16 i = 0; i < size.x(); ++i )
    data.at( i ).resize( size.y() );
}

size_t CustomHouseElements::estimatedSize() const
{
  size_t size = sizeof( CustomHouseElements );
  size += 3 * sizeof( HouseFloor* ) + data.capacity() * sizeof( HouseFloor );
  for ( const auto& floor : data )
  {
    size += 3 * sizeof( std::list<CUSTOM_HOUSE_ELEMENT>* ) +
            floor.capacity() * sizeof( std::list<CUSTOM_HOUSE_ELEMENT> );
    for ( const auto& l : floor )
    {
      size += 3 * sizeof( CUSTOM_HOUSE_ELEMENT* ) + l.size() * sizeof( CUSTOM_HOUSE_ELEMENT );
    }
  }
  return size;
}

HouseFloorZColumn* CustomHouseElements::GetElementsAt( const Core::Vec2d& off )
{
  Core::Pos2d xy = Core::Pos2d( 0, 0 ) + ( off + offset );
  return &( data.at( xy.x() ).at( xy.y() ) );
}
void CustomHouseElements::AddElement( CUSTOM_HOUSE_ELEMENT& elem )
{
  Core::Pos2d xy = Core::Pos2d( 0, 0 ) + ( elem.offset + offset );
  data.at( xy.x() ).at( xy.y() ).push_back( elem );
}
}  // namespace Multi
}  // namespace Pol
