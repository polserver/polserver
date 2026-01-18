#include "testfiles.h"

#include "../clib/clib.h"
#include "../clib/logfacility.h"
#include "../clib/strutil.h"
#include "../plib/mapblock.h"

#include <cstring>
#include <filesystem>

#include <fstream>
#include <utility>
#include <vector>


namespace Pol::PolTool
{
FileGenerator::FileGenerator( fs::path basedir, bool hsa, int maxtiles, int mapid, int width,
                              int height )
    : _basedir( std::move( basedir ) ),
      _hsa( hsa ),
      _maxtiles( maxtiles ),
      _mapid( mapid ),
      _width( width ),
      _height( height )
{
  if ( _width % Plib::MAPBLOCK_CHUNK != 0 || _height % Plib::MAPBLOCK_CHUNK != 0 )
    throw std::runtime_error( std::string( "height and width need to be divisible by " ) +
                              Clib::tostring( Plib::MAPBLOCK_CHUNK ) );
  if ( _basedir.empty() )
    _basedir = ".";
  INFO_PRINTLN(
      "Generating testfiles\n"
      "  basedir: {}\n"
      "  HSA: {}\n"
      "  maxtiles: {:#x}\n"
      "  mapid: {}\n"
      "  width: {}\n"
      "  height: {}",
      _basedir.string(), _hsa, _maxtiles, _mapid, _width, _height );

  if ( _basedir != "." && !fs::exists( _basedir ) )
    fs::create_directories( _basedir );
}

template <typename T>
void FileGenerator::writeFile( std::ofstream& stream, T& data )
{
  stream.write( reinterpret_cast<char*>( &data ), sizeof( T ) );
}

template <typename T, typename U>
void FileGenerator::modifyTiledata( std::vector<T>& land, std::vector<U>& item )
{
  auto addland = []( T* e, u32 flags, std::string name )
  {
    e->flags = flags;
    strncpy( e->name, name.c_str(), 19 );
  };
  auto additem = []( U* e, u32 flags, u8 weight, u8 layer, u8 height, std::string name )
  {
    e->flags = flags;
    e->weight = weight;
    e->layer = layer;
    e->height = height;
    strncpy( e->name, name.c_str(), 19 );
  };

  // flags need to be atleast 1 to be part of the cfg
  /*  for ( auto& l : land )
      l.flags = 1;
    for ( auto& i : item )
      i.flags = 1;
   */
  addland( &land[0x3], 0x04000000, "grass" );
  addland( &land[0xa8], 0x000000c0, "water" );
  additem( &item[0xe75], 0x00600002, 3, 21, 1, "backpack" );
  additem( &item[0xf3f], 0x04000800, 0, 0, 1, "arrow" );
  additem( &item[0xeed], 0x04000801, 0, 0, 0, "gold coin" );

  additem( &item[0x2006], 0x00214000, 255, 0, 0, "corpse" );

  // equipment
  additem( &item[0x1517], 0x00404002, 1, 5, 1, "shirt" );
  additem( &item[0x152e], 0x08400000, 2, 4, 1, "short pants" );
  additem( &item[0x203b], 0x00400002, 0, 11, 0, "short hair" );

  // house 0x6b
  additem( &item[0x6], 0x00002050, 255, 0, 20, "wooden wall" );
  additem( &item[0x7], 0x00002050, 255, 0, 20, "wooden wall" );
  additem( &item[0x8], 0x00002050, 255, 0, 20, "wooden wall" );
  additem( &item[0x9], 0x00002050, 255, 0, 20, "wooden post" );
  additem( &item[0xa], 0x00002050, 255, 0, 20, "wooden wall" );
  additem( &item[0xc], 0x04002050, 255, 0, 20, "wooden wall" );
  additem( &item[0xf], 0x00801050, 255, 40, 20, "window" );
  additem( &item[0x18], 0x04002050, 255, 0, 3, "wooden wall" );
  additem( &item[0x63], 0x00002050, 255, 0, 5, "stone wall" );
  additem( &item[0x64], 0x00002050, 255, 0, 5, "stone wall" );
  additem( &item[0x65], 0x00002050, 255, 0, 5, "stone wall Shirt2_M" );
  additem( &item[0x66], 0x04002050, 255, 0, 5, "stone wall n booksh" );
  additem( &item[0x4a9], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4aa], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4ab], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4ac], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4ad], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4ae], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4af], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4b0], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4b1], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4b2], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4b3], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x4b4], 0x00002201, 255, 0, 0, "wooden boards" );
  additem( &item[0x5c2], 0x10002000, 255, 0, 3, "wooden shingle" );
  additem( &item[0x5c3], 0x14002000, 255, 0, 3, "wooden shingles" );
  additem( &item[0x5c4], 0x14002000, 255, 0, 3, "wooden shingles" );
  additem( &item[0x6a5], 0x20002050, 255, 0, 20, "wooden door" );
  additem( &item[0x751], 0x00002600, 255, 0, 5, "stone stairs" );
  additem( &item[0x756], 0x00002600, 255, 0, 5, "stone stairs" );
  additem( &item[0x758], 0x00002600, 255, 0, 5, "stone stairs" );
  additem( &item[0xb98], 0x00000004, 255, 0, 0, "wooden signpost" );
  additem( &item[0xbd2], 0x00000004, 255, 0, 0, "brass sign" );
  // first boat
  additem( &item[0x3e3f], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3e41], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3e4b], 0x00200040, 255, 0, 3, "tiller man" );
  additem( &item[0x3e4e], 0x00200040, 255, 0, 3, "tiller man" );
  additem( &item[0x3e50], 0x00200040, 255, 0, 3, "tiller man" );
  additem( &item[0x3e55], 0x00200040, 255, 0, 3, "tiller man" );
  additem( &item[0x3e59], 0x00020004, 255, 0, 3, "sail" );
  additem( &item[0x3e5a], 0x00020040, 255, 0, 3, "mast" );
  additem( &item[0x3e5b], 0x00020004, 255, 0, 3, "sail" );
  additem( &item[0x3e5c], 0x00020004, 255, 0, 3, "sail" );
  additem( &item[0x3e63], 0x00000040, 255, 0, 3, "tiller" );
  additem( &item[0x3e65], 0x80200201, 255, 0, 3, "hatch" );
  additem( &item[0x3e66], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3e67], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3e69], 0x00000040, 255, 0, 3, "prow" );
  additem( &item[0x3e6b], 0x00020004, 255, 0, 3, "sail" );
  additem( &item[0x3e6c], 0x00020040, 255, 0, 3, "mast" );
  additem( &item[0x3e6d], 0x00020004, 255, 0, 3, "sail" );
  additem( &item[0x3e6e], 0x00020004, 255, 0, 3, "spar" );
  additem( &item[0x3e76], 0x00000040, 255, 0, 3, "rudder" );
  additem( &item[0x3e79], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3e7b], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3e7c], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3e7d], 0x80000041, 255, 0, 3, "ship" );
  additem( &item[0x3e7e], 0x80000041, 255, 0, 3, "ship" );
  additem( &item[0x3e7f], 0x80000041, 255, 0, 3, "ship" );
  additem( &item[0x3e85], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3e88], 0x80000201, 255, 0, 3, "deck" );
  additem( &item[0x3e8a], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3e8b], 0x80000201, 255, 0, 3, "deck" );
  additem( &item[0x3e8c], 0x80000041, 255, 0, 3, "ship" );
  additem( &item[0x3e8d], 0x80000041, 255, 0, 3, "ship" );
  additem( &item[0x3e8e], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3e8f], 0x80000201, 255, 0, 3, "deck" );
  additem( &item[0x3e90], 0x00000040, 255, 25, 3, "ship" );
  additem( &item[0x3e91], 0x00000040, 255, 25, 3, "ship" );
  additem( &item[0x3e93], 0x80200201, 255, 0, 3, "hatch" );
  additem( &item[0x3e94], 0x00000040, 255, 25, 3, "ship" );
  additem( &item[0x3e95], 0x00000040, 255, 0, 3, "prow" );
  additem( &item[0x3e9a], 0x00000040, 255, 12, 3, "prow" );
  additem( &item[0x3e9b], 0x00000040, 255, 12, 3, "ship" );
  additem( &item[0x3e9c], 0x00000040, 255, 12, 3, "ship" );
  additem( &item[0x3e9d], 0x00000040, 255, 12, 3, "ship" );
  additem( &item[0x3e9e], 0x00000040, 255, 18, 3, "ship" );
  additem( &item[0x3e9f], 0x80000041, 255, 25, 3, "ship" );
  additem( &item[0x3ea0], 0x80000041, 255, 25, 3, "ship" );
  additem( &item[0x3ea1], 0x80000201, 255, 25, 3, "deck" );
  additem( &item[0x3ea5], 0x80000041, 255, 25, 3, "ship" );
  additem( &item[0x3ea6], 0x80000041, 255, 25, 3, "ship" );
  additem( &item[0x3ea7], 0x80000041, 255, 0, 3, "ship" );
  additem( &item[0x3ea8], 0x80000041, 255, 0, 3, "ship" );
  additem( &item[0x3eaa], 0x80000041, 255, 0, 3, "deck" );
  additem( &item[0x3eac], 0x80000201, 255, 25, 3, "deck" );
  additem( &item[0x3ead], 0x80000201, 255, 25, 3, "deck" );
  additem( &item[0x3eae], 0x00200200, 255, 0, 3, "hatch" );
  additem( &item[0x3eb1], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3eb2], 0x00000040, 255, 0, 3, "ship" );
  additem( &item[0x3eb4], 0x00000040, 255, 25, 3, "prow" );
  additem( &item[0x3eb5], 0x00000040, 255, 2, 3, "ship" );
  additem( &item[0x3eb6], 0x00000040, 255, 2, 3, "ship" );
  additem( &item[0x3eb9], 0x00200200, 255, 4, 3, "hatch" );
  additem( &item[0x3ebc], 0x00000040, 255, 25, 3, "rudder" );
  additem( &item[0x3ec4], 0x00000040, 255, 0, 3, "tiller" );
  additem( &item[0x3ece], 0x00020004, 255, 0, 3, "spar" );
  additem( &item[0x3edc], 0x04020004, 255, 0, 1, "sail" );
  additem( &item[0x3edd], 0x00020040, 255, 0, 1, "mast" );
  additem( &item[0x3ede], 0x00020004, 255, 1, 1, "sail" );
  additem( &item[0x3ee0], 0x04020004, 255, 2, 1, "sail" );
  additem( &item[0x3ee1], 0x00020004, 255, 0, 1, "sail" );
  additem( &item[0x3ee2], 0x00020040, 255, 3, 1, "mast" );
  additem( &item[0x3ee3], 0x00020004, 255, 0, 1, "sail" );
  additem( &item[0x3ee4], 0x00000040, 255, 4, 3, "ship" );
  additem( &item[0x3ee6], 0x00000040, 255, 5, 3, "ship" );
  // end first boat
  additem( &item[0x31f4], 0x00000201, 255, 0, 0, "dirt" );

  additem( &item.back(), 1, 2, 3, 4, "test" );
}

template <typename T, typename U>
void FileGenerator::writeTiledata( std::vector<T>& land, std::vector<U>& item )
{
  std::ofstream file( _basedir / "tiledata.mul", std::ofstream::binary | std::ofstream::out );
  int header = 1;
  for ( size_t i = 0; i < land.size(); ++i )
  {
    if ( ( i & 0x1f ) == 0 )
      writeFile( file, header );
    writeFile( file, land[i] );
  }
  for ( size_t i = 0; i < item.size(); ++i )
  {
    if ( ( i & 0x1f ) == 0 )
      writeFile( file, header );
    writeFile( file, item[i] );
  }
}

template <typename T, typename U>
void FileGenerator::genTiledata( std::vector<T>& land, std::vector<U>& item )
{
  modifyTiledata( land, item );
  writeTiledata( land, item );
}

void FileGenerator::generateTiledata()
{
  INFO_PRINTLN( "Generating tiledata.mul" );
  if ( !_hsa )
  {
    std::vector<Plib::USTRUCT_LAND_TILE> landtiles( 0x4000, Plib::USTRUCT_LAND_TILE() );
    std::vector<Plib::USTRUCT_TILE> itemtiles( _maxtiles + 1, Plib::USTRUCT_TILE() );
    genTiledata( landtiles, itemtiles );
  }
  else
  {
    std::vector<Plib::USTRUCT_LAND_TILE_HSA> landtiles( 0x4000, Plib::USTRUCT_LAND_TILE_HSA() );
    std::vector<Plib::USTRUCT_TILE_HSA> itemtiles( _maxtiles + 1, Plib::USTRUCT_TILE_HSA() );
    genTiledata( landtiles, itemtiles );
  }
}

void FileGenerator::generateMap()
{
  INFO_PRINTLN( "Generating map{}.mul", _mapid );
  // initialize with grass tiles on z=0
  Plib::USTRUCT_MAPINFO grass{ 0x3, 0 };
  std::vector<std::vector<Plib::USTRUCT_MAPINFO>> map;
  map.resize( _height );
  for ( auto& line : map )
    line = std::vector<Plib::USTRUCT_MAPINFO>( _width, grass );

  modifyMap( map );

  int header = 0;
  std::ofstream file( _basedir / ( "map" + std::to_string( _mapid ) + ".mul" ),
                      std::ofstream::binary | std::ofstream::out );
  for ( int x = 0; x < _width / 8; ++x )
  {
    for ( int y = 0; y < _height / 8; ++y )
    {
      writeFile( file, header );
      for ( int by = 0; by < 8; ++by )
      {
        for ( int bx = 0; bx < 8; ++bx )
        {
          writeFile( file, map[y * 8 + by][x * 8 + bx] );
        }
      }
    }
  }
}

void FileGenerator::modifyMap( std::vector<std::vector<Plib::USTRUCT_MAPINFO>>& map )
{
  Plib::USTRUCT_MAPINFO water{ 0xa8, -5 };
  Plib::USTRUCT_MAPINFO sandborder{ 0x17, -3 };
  Plib::USTRUCT_MAPINFO sand{ 0x17, 0 };
  const int border = 30;
  int y = 0;
  for ( auto& line : map )
  {
    int x = 0;
    // fill border (without transition area) with water
    std::replace_if(
        line.begin(), line.end(),
        [&]( const auto& )
        {
          if ( x < border - 2 || x > _width - border + 1 || y < border - 2 ||
               y > _height - border + 1 )
          {
            ++x;
            return true;
          }
          ++x;
          return false;
        },
        water );
    // fill outer transition line with sand and z -3
    x = 0;
    std::replace_if(
        line.begin(), line.end(),
        [&]( const auto& e )
        {
          if ( e.landtile != 0x3 )
          {
            ++x;
            return false;
          }
          if ( x == border - 2 || x == _width - border + 1 || y == border - 2 ||
               y == _height - border + 1 )
          {
            ++x;
            return true;
          }
          ++x;
          return false;
        },
        sandborder );
    // fill inner transition line with sand and z 0
    x = 0;
    std::replace_if(
        line.begin(), line.end(),
        [&]( const auto& e )
        {
          if ( e.landtile != 0x3 )
          {
            ++x;
            return false;
          }
          if ( x == border - 1 || x == _width - border || y == border - 1 || y == _height - border )
          {
            ++x;
            return true;
          }
          ++x;
          return false;
        },
        sand );
    ++y;
  }
}

void FileGenerator::generateStatics()
{
  INFO_PRINTLN( "Generating statics{}.mul", _mapid );
  Plib::USTRUCT_IDX idxempty{ 0xFFffFFff, 0xFFffFFff, 0xFFffFFff };
  std::ofstream file( _basedir / ( "statics" + std::to_string( _mapid ) + ".mul" ),
                      std::ofstream::binary | std::ofstream::out );
  std::ofstream fileidx( _basedir / ( "staidx" + std::to_string( _mapid ) + ".mul" ),
                         std::ofstream::binary | std::ofstream::out );

  // init statics: [y][x][...]
  std::vector<std::vector<std::vector<Plib::USTRUCT_STATIC>>> statics;
  statics.resize( _height / 8 );
  for ( auto& line : statics )
  {
    line = std::vector<std::vector<Plib::USTRUCT_STATIC>>( _width / 8,
                                                           std::vector<Plib::USTRUCT_STATIC>() );
  }

  modifyStatics( statics );

  Plib::USTRUCT_IDX idx{ 0, 0, 0xFFffFFff };
  for ( int x = 0; x < _width / 8; ++x )
  {
    for ( int y = 0; y < _height / 8; ++y )
    {
      if ( statics[y][x].empty() )
        writeFile( fileidx, idxempty );
      else
      {
        for ( auto& s : statics[y][x] )
          writeFile( file, s );

        idx.length = static_cast<u32>( sizeof( Plib::USTRUCT_STATIC ) * statics[y][x].size() );
        writeFile( fileidx, idx );
        idx.offset += idx.length;
      }
    }
  }
}

void FileGenerator::modifyStatics(
    std::vector<std::vector<std::vector<Plib::USTRUCT_STATIC>>>& statics )
{
  auto addstatic = [&]( int x, int y, s8 z, u16 graphic, u16 hue )
  {
    statics[y / 8][x / 8].push_back(
        { graphic, static_cast<s8>( x % 8 ), static_cast<s8>( y % 8 ), z, hue } );
  };

  auto addstatic_rel = [&]( u16 graphic, s16 x, s16 y, s16 z, u16 hue = 0 )
  { addstatic( 150 + x, 150 + y, Clib::clamp_convert<s8>( 0 + z ), graphic, hue ); };
  const u16 roof = 0x1;
  addstatic_rel( 0x0066, -3, -3, 0 );
  addstatic_rel( 0x0009, -3, -3, 7 );
  addstatic_rel( 0x0064, -3, -2, 0 );
  addstatic_rel( 0x0008, -3, -2, 7 );
  addstatic_rel( 0x0064, -3, -1, 0 );
  addstatic_rel( 0x0008, -3, -1, 7 );
  addstatic_rel( 0x0064, -3, 0, 0 );
  addstatic_rel( 0x0008, -3, 0, 7 );
  addstatic_rel( 0x0064, -3, 1, 0 );
  addstatic_rel( 0x000f, -3, 1, 7 );
  addstatic_rel( 0x0064, -3, 2, 0 );
  addstatic_rel( 0x0008, -3, 2, 7 );
  addstatic_rel( 0x0064, -3, 3, 0 );
  addstatic_rel( 0x0008, -3, 3, 7 );
  addstatic_rel( 0x0063, -2, -3, 0 );
  addstatic_rel( 0x0007, -2, -3, 7 );
  addstatic_rel( 0x04b4, -2, -2, 7 );
  addstatic_rel( 0x05c4, -2, -2, 27 );
  addstatic_rel( 0x04ad, -2, -1, 7 );
  addstatic_rel( 0x05c4, -2, -1, 27 );
  addstatic_rel( 0x04ad, -2, 0, 7 );
  addstatic_rel( 0x05c4, -2, 0, 27 );
  addstatic_rel( 0x04ad, -2, 1, 7 );
  addstatic_rel( 0x05c4, -2, 1, 27 );
  addstatic_rel( 0x04ad, -2, 2, 7 );
  addstatic_rel( 0x05c4, -2, 2, 27 );
  addstatic_rel( 0x0063, -2, 3, 0 );
  addstatic_rel( 0x04b2, -2, 3, 7 );
  addstatic_rel( 0x0007, -2, 3, 7 );
  addstatic_rel( 0x05c4, -2, 3, 27 );
  addstatic_rel( 0x05c4, -2, 4, 27 );
  addstatic_rel( 0x0063, -1, -3, 0 );
  addstatic_rel( 0x0007, -1, -3, 7 );
  addstatic_rel( 0x04b0, -1, -2, 7 );
  addstatic_rel( 0x05c4, -1, -2, 30 );
  addstatic_rel( 0x04ab, -1, -1, 7 );
  addstatic_rel( 0x05c4, -1, -1, 30 );
  addstatic_rel( 0x04aa, -1, 0, 7 );
  addstatic_rel( 0x05c4, -1, 0, 30 );
  addstatic_rel( 0x04ac, -1, 1, 7 );
  addstatic_rel( 0x05c4, -1, 1, 30 );
  addstatic_rel( 0x04a9, -1, 2, 7 );
  addstatic_rel( 0x05c4, -1, 2, 30 );
  addstatic_rel( 0x0063, -1, 3, 0 );
  addstatic_rel( 0x04ae, -1, 3, 7 );
  addstatic_rel( 0x000c, -1, 3, 7 );
  addstatic_rel( 0x0018, -1, 3, 27 );
  addstatic_rel( 0x05c4, -1, 3, 30 );
  addstatic_rel( 0x0758, -1, 4, 2 );
  addstatic_rel( 0x05c4, -1, 4, 30 );
  addstatic_rel( 0x0063, 0, -3, 0 );
  addstatic_rel( 0x0007, 0, -3, 7 );
  addstatic_rel( 0x04b0, 0, -2, 7 );
  addstatic_rel( 0x05c4, 0, -2, 33 );
  addstatic_rel( 0x04a9, 0, -1, 7 );
  addstatic_rel( 0x05c4, 0, -1, 33 );
  addstatic_rel( 0x04ac, 0, 0, 7 );
  addstatic_rel( 0x05c4, 0, 0, 33 );
  addstatic_rel( 0x04ab, 0, 1, 7 );
  addstatic_rel( 0x05c4, 0, 1, 33 );
  addstatic_rel( 0x04aa, 0, 2, 7 );
  addstatic_rel( 0x05c4, 0, 2, 33 );
  addstatic_rel( 0x04ae, 0, 3, 7 );
  //  addstatic_rel( 0x06a5, 0, 3, 7 );
  addstatic_rel( 0x0018, 0, 3, 27 );
  addstatic_rel( 0x05c4, 0, 3, 33 );
  addstatic_rel( 0x0751, 0, 4, 2 );
  addstatic_rel( 0x05c4, 0, 4, 33 );
  addstatic_rel( 0x0063, 1, -3, 0 );
  addstatic_rel( 0x0007, 1, -3, 7 );
  addstatic_rel( 0x04b0, 1, -2, 7 );
  addstatic_rel( 0x05c2, 1, -2, 36 );
  addstatic_rel( 0x04aa, 1, -1, 7 );
  addstatic_rel( 0x05c2, 1, -1, 36 );
  addstatic_rel( 0x04a9, 1, 0, 7 );
  addstatic_rel( 0x05c2, 1, 0, 36 );
  addstatic_rel( 0x04a9, 1, 1, 7 );
  addstatic_rel( 0x05c2, 1, 1, 36 );
  addstatic_rel( 0x04ab, 1, 2, 7 );
  addstatic_rel( 0x05c2, 1, 2, 36 );
  addstatic_rel( 0x0063, 1, 3, 0 );
  addstatic_rel( 0x04ae, 1, 3, 7 );
  addstatic_rel( 0x000a, 1, 3, 7 );
  addstatic_rel( 0x0018, 1, 3, 27 );
  addstatic_rel( 0x05c2, 1, 3, 36 );
  addstatic_rel( 0x0756, 1, 4, 2 );
  addstatic_rel( 0x05c2, 1, 4, 36 );
  addstatic_rel( 0x0063, 2, -3, 0 );
  addstatic_rel( 0x0007, 2, -3, 7 );
  addstatic_rel( 0x04b0, 2, -2, 7 );
  addstatic_rel( 0x05c3, 2, -2, 33, roof );
  addstatic_rel( 0x04ac, 2, -1, 7 );
  addstatic_rel( 0x05c3, 2, -1, 33 );
  addstatic_rel( 0x04ab, 2, 0, 7 );
  addstatic_rel( 0x05c3, 2, 0, 33 );
  addstatic_rel( 0x04aa, 2, 1, 7 );
  addstatic_rel( 0x05c3, 2, 1, 33 );
  addstatic_rel( 0x04ac, 2, 2, 7 );
  addstatic_rel( 0x05c3, 2, 2, 33 );
  addstatic_rel( 0x0063, 2, 3, 0 );
  addstatic_rel( 0x04ae, 2, 3, 7 );
  addstatic_rel( 0x0007, 2, 3, 7 );
  addstatic_rel( 0x05c3, 2, 3, 33, roof );
  // addstatic_rel( 0x0bd2, 2, 4, 5 );
  addstatic_rel( 0x0b98, 2, 4, 5 );
  addstatic_rel( 0x05c3, 2, 4, 33, roof );
  addstatic_rel( 0x0063, 3, -3, 0 );
  addstatic_rel( 0x0007, 3, -3, 7 );
  addstatic_rel( 0x0064, 3, -2, 0 );
  addstatic_rel( 0x04b3, 3, -2, 7 );
  addstatic_rel( 0x0008, 3, -2, 7 );
  addstatic_rel( 0x05c3, 3, -2, 30, roof );
  addstatic_rel( 0x0064, 3, -1, 0 );
  addstatic_rel( 0x04af, 3, -1, 7 );
  addstatic_rel( 0x0008, 3, -1, 7 );
  addstatic_rel( 0x05c3, 3, -1, 30 );
  addstatic_rel( 0x0064, 3, 0, 0 );
  addstatic_rel( 0x04af, 3, 0, 7 );
  addstatic_rel( 0x0008, 3, 0, 7 );
  addstatic_rel( 0x05c3, 3, 0, 30, roof );
  addstatic_rel( 0x0064, 3, 1, 0 );
  addstatic_rel( 0x04af, 3, 1, 7 );
  addstatic_rel( 0x000f, 3, 1, 7 );
  addstatic_rel( 0x05c3, 3, 1, 30, roof );
  addstatic_rel( 0x0064, 3, 2, 0 );
  addstatic_rel( 0x04af, 3, 2, 7 );
  addstatic_rel( 0x0008, 3, 2, 7 );
  addstatic_rel( 0x05c3, 3, 2, 30 );
  addstatic_rel( 0x0065, 3, 3, 0 );
  addstatic_rel( 0x04b1, 3, 3, 7 );
  addstatic_rel( 0x0006, 3, 3, 7 );
  addstatic_rel( 0x05c3, 3, 3, 30, roof );
  addstatic_rel( 0x05c3, 3, 4, 30, roof );
  addstatic_rel( 0x05c3, 4, -2, 27, roof );
  addstatic_rel( 0x05c3, 4, -1, 27 );
  addstatic_rel( 0x05c3, 4, 0, 27, roof );
  addstatic_rel( 0x05c3, 4, 1, 27, roof );
  addstatic_rel( 0x05c3, 4, 2, 27 );
  addstatic_rel( 0x05c3, 4, 3, 27 );
  addstatic_rel( 0x05c3, 4, 4, 27, roof );
}

template <typename T>
void FileGenerator::modifyMultis( std::vector<std::vector<T>>& multis )
{
  auto elem = [&]( u16 graphic, s16 x, s16 y, s16 z, u32 flags )
  {
    T e{};
    e.graphic = graphic;
    e.x = x;
    e.y = y;
    e.z = z;
    e.flags = flags;
    return e;
  };
  multis.resize( 0x3fff, std::vector<T>() );
  for ( int alternate_offset = 0; alternate_offset < 12; alternate_offset += 4 )
  {
    // boat
    multis[0x0 + alternate_offset] = std::vector<T>{
        elem( 0x3edd, 0, 0, 0, 1 ),   elem( 0x3e4e, 1, 4, 0, 0 ),   elem( 0x3eae, 0, -4, 0, 0 ),
        elem( 0x3eb2, 2, 0, 0, 0 ),   elem( 0x3eb1, -2, 0, 0, 0 ),  elem( 0x3eac, 0, -2, 0, 1 ),
        elem( 0x3e9e, 1, -3, 0, 1 ),  elem( 0x3ead, 0, -3, 0, 1 ),  elem( 0x3e9d, -1, -3, 0, 1 ),
        elem( 0x3eac, 1, -1, 0, 1 ),  elem( 0x3ea1, -1, -1, 0, 1 ), elem( 0x3ea0, 1, -2, 0, 1 ),
        elem( 0x3e9f, -1, -2, 0, 1 ), elem( 0x3eac, 0, -1, 0, 1 ),  elem( 0x3ea1, -1, 0, 0, 1 ),
        elem( 0x3eac, 1, 0, 0, 1 ),   elem( 0x3e9c, 1, -4, 0, 1 ),  elem( 0x3e9b, -1, -4, 0, 1 ),
        elem( 0x3ee4, 2, -1, 0, 1 ),  elem( 0x3eb1, -2, -1, 0, 1 ), elem( 0x3e9a, 0, -5, 0, 1 ),
        elem( 0x3ede, 2, 0, 0, 1 ),   elem( 0x3edc, -1, 1, 0, 1 ),  elem( 0x3ea1, -1, 1, 0, 1 ),
        elem( 0x3eac, 0, 1, 0, 1 ),   elem( 0x3eac, 1, 1, 0, 1 ),   elem( 0x3ea5, -1, 2, 0, 1 ),
        elem( 0x3eac, 0, 2, 0, 1 ),   elem( 0x3ea6, 1, 2, 0, 1 ),   elem( 0x3eb2, 2, 1, 0, 1 ),
        elem( 0x3eb1, -2, 1, 0, 1 ),  elem( 0x3ece, -2, 2, 0, 1 ),  elem( 0x3ea7, -1, 3, 0, 1 ),
        elem( 0x3eac, 0, 3, 0, 1 ),   elem( 0x3ea8, 1, 3, 0, 1 ),   elem( 0x3eb6, -1, 4, 0, 1 ),
        elem( 0x3eaa, 0, 4, 0, 1 ),   elem( 0x3ebc, 0, 5, 0, 1 ),
    };
    multis[0x1 + alternate_offset] = std::vector<T>{
        elem( 0x3e5a, 0, 0, 0, 1 ),   elem( 0x3e55, -4, 0, 0, 0 ),  elem( 0x3e65, 4, 0, 0, 0 ),
        elem( 0x3e85, 0, 2, 0, 0 ),   elem( 0x3e8a, 0, -2, 0, 0 ),  elem( 0x3e88, 0, -1, 0, 1 ),
        elem( 0x3e8b, 1, 1, 0, 1 ),   elem( 0x3e8b, -1, 1, 0, 1 ),  elem( 0x3e8b, 0, 1, 0, 1 ),
        elem( 0x3e8b, 1, 0, 0, 1 ),   elem( 0x3e5b, 1, -1, 0, 1 ),  elem( 0x3e88, 1, -1, 0, 1 ),
        elem( 0x3e8b, -1, 0, 0, 1 ),  elem( 0x3e88, -1, -1, 0, 1 ), elem( 0x3e8a, -1, -2, 0, 1 ),
        elem( 0x3e8b, -2, 0, 0, 1 ),  elem( 0x3e8d, -2, -1, 0, 1 ), elem( 0x3e59, 0, 2, 0, 1 ),
        elem( 0x3e3f, -1, 2, 0, 1 ),  elem( 0x3e7e, 2, 1, 0, 1 ),   elem( 0x3e8a, 1, -2, 0, 1 ),
        elem( 0x3e7f, 2, -1, 0, 1 ),  elem( 0x3e8b, 2, 0, 0, 1 ),   elem( 0x3e85, 1, 2, 0, 1 ),
        elem( 0x3e8c, -2, 1, 0, 1 ),  elem( 0x3e8b, 3, 0, 0, 1 ),   elem( 0x3e7d, 3, -1, 0, 1 ),
        elem( 0x3e90, -3, -1, 0, 1 ), elem( 0x3e8f, -3, 0, 0, 1 ),  elem( 0x3e5c, 2, -2, 0, 1 ),
        elem( 0x3e7c, 3, 1, 0, 1 ),   elem( 0x3e8e, -3, 1, 0, 1 ),  elem( 0x3e66, 4, 1, 0, 1 ),
        elem( 0x3e67, 4, -1, 0, 1 ),  elem( 0x3e91, -4, 1, 0, 1 ),  elem( 0x3e94, -4, -1, 0, 1 ),
        elem( 0x3e63, -5, 0, 0, 1 ),  elem( 0x3e69, 5, 0, 0, 1 ),
    };
    multis[0x2 + alternate_offset] = std::vector<T>{
        elem( 0x3ee2, 0, 0, 0, 1 ),   elem( 0x3e4b, 0, -4, 0, 0 ),  elem( 0x3eb9, 0, 4, 0, 0 ),
        elem( 0x3eb1, -2, 0, 0, 0 ),  elem( 0x3eb2, 2, 0, 0, 0 ),   elem( 0x3eac, 1, 0, 0, 1 ),
        elem( 0x3eac, 1, -1, 0, 1 ),  elem( 0x3eac, 1, 1, 0, 1 ),   elem( 0x3eac, 0, 1, 0, 1 ),
        elem( 0x3eac, 0, -1, 0, 1 ),  elem( 0x3ee1, -1, 1, 0, 1 ),  elem( 0x3ea1, -1, -1, 0, 1 ),
        elem( 0x3ea1, -1, 0, 0, 1 ),  elem( 0x3ea1, -1, 1, 0, 1 ),  elem( 0x3eb1, -2, 1, 0, 1 ),
        elem( 0x3eb1, -2, -1, 0, 1 ), elem( 0x3ea0, 1, -2, 0, 1 ),  elem( 0x3e9f, -1, -2, 0, 1 ),
        elem( 0x3eac, 0, -2, 0, 1 ),  elem( 0x3ee3, 2, 0, 0, 1 ),   elem( 0x3ee6, 2, -1, 0, 1 ),
        elem( 0x3eb2, 2, 1, 0, 1 ),   elem( 0x3ea6, 1, 2, 0, 1 ),   elem( 0x3eac, 0, 2, 0, 1 ),
        elem( 0x3ea5, -1, 2, 0, 1 ),  elem( 0x3ead, 0, -3, 0, 1 ),  elem( 0x3e9d, -1, -3, 0, 1 ),
        elem( 0x3e9e, 1, -3, 0, 1 ),  elem( 0x3ee0, -2, 2, 0, 1 ),  elem( 0x3ea8, 1, 3, 0, 1 ),
        elem( 0x3ea7, -1, 3, 0, 1 ),  elem( 0x3eac, 0, 3, 0, 1 ),   elem( 0x3eb6, -1, 4, 0, 1 ),
        elem( 0x3eb5, 1, 4, 0, 1 ),   elem( 0x3e9b, -1, -4, 0, 1 ), elem( 0x3e9c, 1, -4, 0, 1 ),
        elem( 0x3eb4, 0, 5, 0, 1 ),   elem( 0x3ec4, 0, -5, 0, 1 ),
    };
    multis[0x3 + alternate_offset] = std::vector<T>{
        elem( 0x3e6c, 0, 0, 0, 1 ),   elem( 0x3e50, 4, 0, 0, 0 ),   elem( 0x3e93, -4, 0, 0, 0 ),
        elem( 0x3e8a, 0, -2, 0, 0 ),  elem( 0x3e85, 0, 2, 0, 0 ),   elem( 0x3e8b, 1, 1, 0, 1 ),
        elem( 0x3e8b, -1, 1, 0, 1 ),  elem( 0x3e8b, -1, 0, 0, 1 ),  elem( 0x3e88, -1, -1, 0, 1 ),
        elem( 0x3e6d, 1, -1, 0, 1 ),  elem( 0x3e8b, 0, 1, 0, 1 ),   elem( 0x3e88, 1, -1, 0, 1 ),
        elem( 0x3e88, 0, -1, 0, 1 ),  elem( 0x3e8b, 1, 0, 0, 1 ),   elem( 0x3e8b, 2, 0, 0, 1 ),
        elem( 0x3e7e, 2, 1, 0, 1 ),   elem( 0x3e85, 1, 2, 0, 1 ),   elem( 0x3e8a, 1, -2, 0, 1 ),
        elem( 0x3e8a, -1, -2, 0, 1 ), elem( 0x3e8c, -2, 1, 0, 1 ),  elem( 0x3e8b, -2, 0, 0, 1 ),
        elem( 0x3e8d, -2, -1, 0, 1 ), elem( 0x3e6b, 0, 2, 0, 1 ),   elem( 0x3e41, -1, 2, 0, 1 ),
        elem( 0x3e7f, 2, -1, 0, 1 ),  elem( 0x3e7d, 3, -1, 0, 1 ),  elem( 0x3e8b, 3, 0, 0, 1 ),
        elem( 0x3e7c, 3, 1, 0, 1 ),   elem( 0x3e6e, 2, -2, 0, 1 ),  elem( 0x3e8e, -3, 1, 0, 1 ),
        elem( 0x3e8f, -3, 0, 0, 1 ),  elem( 0x3e90, -3, -1, 0, 1 ), elem( 0x3e79, 4, 1, 0, 1 ),
        elem( 0x3e7b, 4, -1, 0, 1 ),  elem( 0x3e94, -4, -1, 0, 1 ), elem( 0x3e91, -4, 1, 0, 1 ),
        elem( 0x3e76, 5, 0, 0, 1 ),   elem( 0x3e95, -5, 0, 0, 1 ),
    };
    // end boat
  }

  // A square boat, with mast in center, deck around mast, sails around deck
  for ( int direction = 0; direction < 4; direction++ )
  {
    multis[0x12 + direction] = std::vector<T>{
        elem( 0x3e6c, 0, 0, 0, 1 ),   elem( 0x3e8b, -1, -1, 0, 1 ), elem( 0x3e8b, -1, 0, 0, 1 ),
        elem( 0x3e8b, -1, 1, 0, 1 ),  elem( 0x3e8b, 0, -1, 0, 1 ),  elem( 0x3e8b, 0, 1, 0, 1 ),
        elem( 0x3e8b, 1, -1, 0, 1 ),  elem( 0x3e8b, 1, 0, 0, 1 ),   elem( 0x3e8b, 1, 1, 0, 1 ),
        elem( 0x3e59, -2, -2, 0, 1 ), elem( 0x3e59, -2, -1, 0, 1 ), elem( 0x3e59, -2, 0, 0, 1 ),
        elem( 0x3e59, -2, 1, 0, 1 ),  elem( 0x3e59, -2, 2, 0, 1 ),  elem( 0x3e59, -1, -2, 0, 1 ),
        elem( 0x3e59, -1, 2, 0, 1 ),  elem( 0x3e59, 0, -2, 0, 1 ),  elem( 0x3e59, 0, 2, 0, 1 ),
        elem( 0x3e59, 1, -2, 0, 1 ),  elem( 0x3e59, 1, 2, 0, 1 ),   elem( 0x3e59, 2, -2, 0, 1 ),
        elem( 0x3e59, 2, -1, 0, 1 ),  elem( 0x3e59, 2, 0, 0, 1 ),   elem( 0x3e59, 2, 1, 0, 1 ),
        elem( 0x3e59, 2, 2, 0, 1 ),
    };
  }

  multis[0x6b] = std::vector<T>{
      elem( 0x0066, -3, -3, 0, 1 ),  elem( 0x0009, -3, -3, 7, 1 ),  elem( 0x0064, -3, -2, 0, 1 ),
      elem( 0x0008, -3, -2, 7, 1 ),  elem( 0x0064, -3, -1, 0, 1 ),  elem( 0x0008, -3, -1, 7, 1 ),
      elem( 0x0064, -3, 0, 0, 1 ),   elem( 0x0008, -3, 0, 7, 1 ),   elem( 0x0064, -3, 1, 0, 1 ),
      elem( 0x000f, -3, 1, 7, 1 ),   elem( 0x0064, -3, 2, 0, 1 ),   elem( 0x0008, -3, 2, 7, 1 ),
      elem( 0x0064, -3, 3, 0, 1 ),   elem( 0x0008, -3, 3, 7, 1 ),   elem( 0x0063, -2, -3, 0, 1 ),
      elem( 0x0007, -2, -3, 7, 1 ),  elem( 0x04b4, -2, -2, 7, 1 ),  elem( 0x05c4, -2, -2, 27, 1 ),
      elem( 0x04ad, -2, -1, 7, 1 ),  elem( 0x05c4, -2, -1, 27, 1 ), elem( 0x04ad, -2, 0, 7, 1 ),
      elem( 0x05c4, -2, 0, 27, 1 ),  elem( 0x04ad, -2, 1, 7, 1 ),   elem( 0x05c4, -2, 1, 27, 1 ),
      elem( 0x04ad, -2, 2, 7, 1 ),   elem( 0x05c4, -2, 2, 27, 1 ),  elem( 0x0063, -2, 3, 0, 1 ),
      elem( 0x04b2, -2, 3, 7, 1 ),   elem( 0x0007, -2, 3, 7, 1 ),   elem( 0x05c4, -2, 3, 27, 1 ),
      elem( 0x05c4, -2, 4, 27, 1 ),  elem( 0x0063, -1, -3, 0, 1 ),  elem( 0x0007, -1, -3, 7, 1 ),
      elem( 0x04b0, -1, -2, 7, 1 ),  elem( 0x05c4, -1, -2, 30, 1 ), elem( 0x04ab, -1, -1, 7, 1 ),
      elem( 0x05c4, -1, -1, 30, 1 ), elem( 0x04aa, -1, 0, 7, 1 ),   elem( 0x05c4, -1, 0, 30, 1 ),
      elem( 0x04ac, -1, 1, 7, 1 ),   elem( 0x05c4, -1, 1, 30, 1 ),  elem( 0x04a9, -1, 2, 7, 1 ),
      elem( 0x05c4, -1, 2, 30, 1 ),  elem( 0x0063, -1, 3, 0, 1 ),   elem( 0x04ae, -1, 3, 7, 1 ),
      elem( 0x000c, -1, 3, 7, 1 ),   elem( 0x0018, -1, 3, 27, 1 ),  elem( 0x05c4, -1, 3, 30, 1 ),
      elem( 0x0758, -1, 4, 2, 1 ),   elem( 0x05c4, -1, 4, 30, 1 ),  elem( 0x0063, 0, -3, 0, 1 ),
      elem( 0x0007, 0, -3, 7, 1 ),   elem( 0x04b0, 0, -2, 7, 1 ),   elem( 0x05c4, 0, -2, 33, 1 ),
      elem( 0x04a9, 0, -1, 7, 1 ),   elem( 0x05c4, 0, -1, 33, 1 ),  elem( 0x04ac, 0, 0, 7, 1 ),
      elem( 0x05c4, 0, 0, 33, 1 ),   elem( 0x04ab, 0, 1, 7, 1 ),    elem( 0x05c4, 0, 1, 33, 1 ),
      elem( 0x04aa, 0, 2, 7, 1 ),    elem( 0x05c4, 0, 2, 33, 1 ),   elem( 0x04ae, 0, 3, 7, 1 ),
      elem( 0x06a5, 0, 3, 7, 0 ),    elem( 0x0018, 0, 3, 27, 1 ),   elem( 0x05c4, 0, 3, 33, 1 ),
      elem( 0x0751, 0, 4, 2, 1 ),    elem( 0x05c4, 0, 4, 33, 1 ),   elem( 0x0063, 1, -3, 0, 1 ),
      elem( 0x0007, 1, -3, 7, 1 ),   elem( 0x04b0, 1, -2, 7, 1 ),   elem( 0x05c2, 1, -2, 36, 1 ),
      elem( 0x04aa, 1, -1, 7, 1 ),   elem( 0x05c2, 1, -1, 36, 1 ),  elem( 0x04a9, 1, 0, 7, 1 ),
      elem( 0x05c2, 1, 0, 36, 1 ),   elem( 0x04a9, 1, 1, 7, 1 ),    elem( 0x05c2, 1, 1, 36, 1 ),
      elem( 0x04ab, 1, 2, 7, 1 ),    elem( 0x05c2, 1, 2, 36, 1 ),   elem( 0x0063, 1, 3, 0, 1 ),
      elem( 0x04ae, 1, 3, 7, 1 ),    elem( 0x000a, 1, 3, 7, 1 ),    elem( 0x0018, 1, 3, 27, 1 ),
      elem( 0x05c2, 1, 3, 36, 1 ),   elem( 0x0756, 1, 4, 2, 1 ),    elem( 0x05c2, 1, 4, 36, 1 ),
      elem( 0x0063, 2, -3, 0, 1 ),   elem( 0x0007, 2, -3, 7, 1 ),   elem( 0x04b0, 2, -2, 7, 1 ),
      elem( 0x05c3, 2, -2, 33, 1 ),  elem( 0x04ac, 2, -1, 7, 1 ),   elem( 0x05c3, 2, -1, 33, 1 ),
      elem( 0x04ab, 2, 0, 7, 1 ),    elem( 0x05c3, 2, 0, 33, 1 ),   elem( 0x04aa, 2, 1, 7, 1 ),
      elem( 0x05c3, 2, 1, 33, 1 ),   elem( 0x04ac, 2, 2, 7, 1 ),    elem( 0x05c3, 2, 2, 33, 1 ),
      elem( 0x0063, 2, 3, 0, 1 ),    elem( 0x04ae, 2, 3, 7, 1 ),    elem( 0x0007, 2, 3, 7, 1 ),
      elem( 0x05c3, 2, 3, 33, 1 ),   elem( 0x0bd2, 2, 4, 5, 0 ),    elem( 0x0b98, 2, 4, 5, 1 ),
      elem( 0x05c3, 2, 4, 33, 1 ),   elem( 0x0063, 3, -3, 0, 1 ),   elem( 0x0007, 3, -3, 7, 1 ),
      elem( 0x0064, 3, -2, 0, 1 ),   elem( 0x04b3, 3, -2, 7, 1 ),   elem( 0x0008, 3, -2, 7, 1 ),
      elem( 0x05c3, 3, -2, 30, 1 ),  elem( 0x0064, 3, -1, 0, 1 ),   elem( 0x04af, 3, -1, 7, 1 ),
      elem( 0x0008, 3, -1, 7, 1 ),   elem( 0x05c3, 3, -1, 30, 1 ),  elem( 0x0064, 3, 0, 0, 1 ),
      elem( 0x04af, 3, 0, 7, 1 ),    elem( 0x0008, 3, 0, 7, 1 ),    elem( 0x05c3, 3, 0, 30, 1 ),
      elem( 0x0064, 3, 1, 0, 1 ),    elem( 0x04af, 3, 1, 7, 1 ),    elem( 0x000f, 3, 1, 7, 1 ),
      elem( 0x05c3, 3, 1, 30, 1 ),   elem( 0x0064, 3, 2, 0, 1 ),    elem( 0x04af, 3, 2, 7, 1 ),
      elem( 0x0008, 3, 2, 7, 1 ),    elem( 0x05c3, 3, 2, 30, 1 ),   elem( 0x0065, 3, 3, 0, 1 ),
      elem( 0x04b1, 3, 3, 7, 1 ),    elem( 0x0006, 3, 3, 7, 1 ),    elem( 0x05c3, 3, 3, 30, 1 ),
      elem( 0x05c3, 3, 4, 30, 1 ),   elem( 0x05c3, 4, -2, 27, 1 ),  elem( 0x05c3, 4, -1, 27, 1 ),
      elem( 0x05c3, 4, 0, 27, 1 ),   elem( 0x05c3, 4, 1, 27, 1 ),   elem( 0x05c3, 4, 2, 27, 1 ),
      elem( 0x05c3, 4, 3, 27, 1 ),   elem( 0x05c3, 4, 4, 27, 1 ),
  };
  multis[0x6c] = std::vector<T>{
      elem( 0x04ae, -30, 0, 0, 1 ), elem( 0x04ae, -29, 0, 0, 1 ), elem( 0x04ae, -28, 0, 0, 1 ),
      elem( 0x04ae, -27, 0, 0, 1 ), elem( 0x04ae, -26, 0, 0, 1 ), elem( 0x04ae, -25, 0, 0, 1 ),
      elem( 0x04ae, -24, 0, 0, 1 ), elem( 0x04ae, -23, 0, 0, 1 ), elem( 0x04ae, -22, 0, 0, 1 ),
      elem( 0x04ae, -21, 0, 0, 1 ), elem( 0x04ae, -20, 0, 0, 1 ), elem( 0x04ae, -19, 0, 0, 1 ),
      elem( 0x04ae, -18, 0, 0, 1 ), elem( 0x04ae, -17, 0, 0, 1 ), elem( 0x04ae, -16, 0, 0, 1 ),
      elem( 0x04ae, -15, 0, 0, 1 ), elem( 0x04ae, -14, 0, 0, 1 ), elem( 0x04ae, -13, 0, 0, 1 ),
      elem( 0x04ae, -12, 0, 0, 1 ), elem( 0x04ae, -11, 0, 0, 1 ), elem( 0x04ae, -10, 0, 0, 1 ),
      elem( 0x04ae, -9, 0, 0, 1 ),  elem( 0x04ae, -8, 0, 0, 1 ),  elem( 0x04ae, -7, 0, 0, 1 ),
      elem( 0x04ae, -6, 0, 0, 1 ),  elem( 0x04ae, -5, 0, 0, 1 ),  elem( 0x04ae, -4, 0, 0, 1 ),
      elem( 0x04ae, -3, 0, 0, 1 ),  elem( 0x04ae, -2, 0, 0, 1 ),  elem( 0x04ae, -1, 0, 0, 1 ),
      elem( 0x04ae, 0, 0, 0, 1 ),   elem( 0x04ae, 1, 0, 0, 1 ),   elem( 0x04ae, 2, 0, 0, 1 ),
      elem( 0x04ae, 3, 0, 0, 1 ),   elem( 0x04ae, 4, 0, 0, 1 ),   elem( 0x04ae, 5, 0, 0, 1 ),
      elem( 0x04ae, 6, 0, 0, 1 ),   elem( 0x04ae, 7, 0, 0, 1 ),   elem( 0x04ae, 8, 0, 0, 1 ),
      elem( 0x04ae, 9, 0, 0, 1 ),   elem( 0x04ae, 10, 0, 0, 1 ),  elem( 0x04ae, 11, 0, 0, 1 ),
      elem( 0x04ae, 12, 0, 0, 1 ),  elem( 0x04ae, 13, 0, 0, 1 ),  elem( 0x04ae, 14, 0, 0, 1 ),
      elem( 0x04ae, 15, 0, 0, 1 ),  elem( 0x04ae, 16, 0, 0, 1 ),  elem( 0x04ae, 17, 0, 0, 1 ),
      elem( 0x04ae, 18, 0, 0, 1 ),  elem( 0x04ae, 19, 0, 0, 1 ),  elem( 0x04ae, 20, 0, 0, 1 ),
      elem( 0x04ae, 21, 0, 0, 1 ),  elem( 0x04ae, 22, 0, 0, 1 ),  elem( 0x04ae, 23, 0, 0, 1 ),
      elem( 0x04ae, 24, 0, 0, 1 ),  elem( 0x04ae, 25, 0, 0, 1 ),  elem( 0x04ae, 26, 0, 0, 1 ),
      elem( 0x04ae, 27, 0, 0, 1 ),  elem( 0x04ae, 28, 0, 0, 1 ),  elem( 0x04ae, 29, 0, 0, 1 ) };

  // A 6x6 dirt plot with a missing 2x2 hole in the northwest
  multis[0x13ea] = std::vector<T>{
      elem( 0x31F4, -2, -2, 0, 1 ), elem( 0x31F4, -2, -1, 0, 1 ), elem( 0x31F4, -2, 0, 0, 1 ),
      elem( 0x31F4, -2, 1, 0, 1 ),  elem( 0x31F4, -2, 2, 0, 1 ),  elem( 0x31F4, -2, 3, 0, 1 ),
      elem( 0x31F4, -1, -2, 0, 1 ), elem( 0x31F4, -1, 1, 0, 1 ),  elem( 0x31F4, -1, 2, 0, 1 ),
      elem( 0x31F4, -1, 3, 0, 1 ),  elem( 0x31F4, 0, -2, 0, 1 ),  elem( 0x31F4, 0, 1, 0, 1 ),
      elem( 0x31F4, 0, 2, 0, 1 ),   elem( 0x31F4, 0, 3, 0, 1 ),   elem( 0x31F4, 1, -2, 0, 1 ),
      elem( 0x31F4, 1, -1, 0, 1 ),  elem( 0x31F4, 1, 0, 0, 1 ),   elem( 0x31F4, 1, 1, 0, 1 ),
      elem( 0x31F4, 1, 2, 0, 1 ),   elem( 0x31F4, 1, 3, 0, 1 ),   elem( 0x31F4, 2, -2, 0, 1 ),
      elem( 0x31F4, 2, -1, 0, 1 ),  elem( 0x31F4, 2, 0, 0, 1 ),   elem( 0x31F4, 2, 1, 0, 1 ),
      elem( 0x31F4, 2, 2, 0, 1 ),   elem( 0x31F4, 2, 3, 0, 1 ),   elem( 0x31F4, 3, -2, 0, 1 ),
      elem( 0x31F4, 3, -1, 0, 1 ),  elem( 0x31F4, 3, 0, 0, 1 ),   elem( 0x31F4, 3, 1, 0, 1 ),
      elem( 0x31F4, 3, 2, 0, 1 ),   elem( 0x31F4, 3, 3, 0, 1 ) };

  // A 6x6 dirt plot with a missing 2x2 hole in the southeast
  multis[0x13eb] = std::vector<T>{
      elem( 0x31F4, -2, -2, 0, 1 ), elem( 0x31F4, -2, -1, 0, 1 ), elem( 0x31F4, -2, 0, 0, 1 ),
      elem( 0x31F4, -2, 1, 0, 1 ),  elem( 0x31F4, -2, 2, 0, 1 ),  elem( 0x31F4, -2, 3, 0, 1 ),
      elem( 0x31F4, -1, -2, 0, 1 ), elem( 0x31F4, -1, -1, 0, 1 ), elem( 0x31F4, -1, 0, 0, 1 ),
      elem( 0x31F4, -1, 1, 0, 1 ),  elem( 0x31F4, -1, 2, 0, 1 ),  elem( 0x31F4, -1, 3, 0, 1 ),
      elem( 0x31F4, 0, -2, 0, 1 ),  elem( 0x31F4, 0, -1, 0, 1 ),  elem( 0x31F4, 0, 0, 0, 1 ),
      elem( 0x31F4, 0, 1, 0, 1 ),   elem( 0x31F4, 0, 2, 0, 1 ),   elem( 0x31F4, 0, 3, 0, 1 ),
      elem( 0x31F4, 1, -2, 0, 1 ),  elem( 0x31F4, 1, -1, 0, 1 ),  elem( 0x31F4, 1, 0, 0, 1 ),
      elem( 0x31F4, 1, 3, 0, 1 ),   elem( 0x31F4, 2, -2, 0, 1 ),  elem( 0x31F4, 2, -1, 0, 1 ),
      elem( 0x31F4, 2, 0, 0, 1 ),   elem( 0x31F4, 2, 3, 0, 1 ),   elem( 0x31F4, 3, -2, 0, 1 ),
      elem( 0x31F4, 3, -1, 0, 1 ),  elem( 0x31F4, 3, 0, 0, 1 ),   elem( 0x31F4, 3, 1, 0, 1 ),
      elem( 0x31F4, 3, 2, 0, 1 ),   elem( 0x31F4, 3, 3, 0, 1 ) };
}

template <typename T>
void FileGenerator::writeMultis( std::vector<std::vector<T>>& multis )
{
  Plib::USTRUCT_IDX idx{ 0, 0, 0xFFffFFff };
  Plib::USTRUCT_IDX idxempty{ 0xFFffFFff, 0xFFffFFff, 0xFFffFFff };
  std::ofstream file( _basedir / "multi.mul", std::ofstream::binary | std::ofstream::out );
  std::ofstream fileidx( _basedir / "multi.idx", std::ofstream::binary | std::ofstream::out );
  idx.offset = 0;
  for ( auto& multi : multis )
  {
    idx.length = 0;
    for ( auto& elem : multi )
    {
      writeFile( file, elem );
      idx.length += sizeof( T );
    }
    if ( multi.empty() )
      writeFile( fileidx, idxempty );
    else
      writeFile( fileidx, idx );
    idx.offset += idx.length;
  }
}

void FileGenerator::generateMultis()
{
  INFO_PRINTLN( "Generating multi.mul" );
  if ( !_hsa )
  {
    std::vector<std::vector<Plib::USTRUCT_MULTI_ELEMENT>> multis;
    modifyMultis( multis );
    writeMultis( multis );
  }
  else
  {
    std::vector<std::vector<Plib::USTRUCT_MULTI_ELEMENT_HSA>> multis;
    modifyMultis( multis );
    writeMultis( multis );
  }
}
}  // namespace Pol::PolTool
