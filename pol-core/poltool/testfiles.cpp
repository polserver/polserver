#include "testfiles.h"

#include "../clib/logfacility.h"
#include "../clib/strutil.h"
#include "../plib/mapblock.h"

#include <cstring>
#include <filesystem>
#include <format/format.h>
#include <fstream>
#include <utility>
#include <vector>

namespace Pol
{
namespace PolTool
{
FileGenerator::FileGenerator( fs::path basedir, bool hsa, int maxtiles, int width, int height )
    : _basedir( std::move( basedir ) ),
      _hsa( hsa ),
      _maxtiles( maxtiles ),
      _width( width ),
      _height( height )
{
  if ( _width % Plib::MAPBLOCK_CHUNK != 0 || _height % Plib::MAPBLOCK_CHUNK != 0 )
    throw std::runtime_error( std::string( "height and width need to be divisible by " ) +
                              Clib::tostring( Plib::MAPBLOCK_CHUNK ) );
  if ( _basedir.empty() )
    _basedir = ".";
  INFO_PRINT << "Generating testfiles\n"
             << "  basedir: " << _basedir.string() << "\n"
             << "  HSA: " << _hsa << "\n"
             << "  maxtiles: 0x" << fmt::hex( _maxtiles ) << "\n"
             << "  width: " << _width << "\n"
             << "  height: " << _height << "\n";

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
  auto addland = []( T* e, u32 flags, std::string name ) {
    e->flags = flags;
    strncpy( e->name, name.c_str(), 19 );
  };
  auto additem = []( U* e, u32 flags, u8 weight, u8 layer, u8 height, std::string name ) {
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
  INFO_PRINT << "Generating tiledata.mul\n";
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
  INFO_PRINT << "Generating map0.mul\n";
  // initialize with grass tiles on z=0
  Plib::USTRUCT_MAPINFO grass{0x3, 0};
  std::vector<std::vector<Plib::USTRUCT_MAPINFO>> map;
  map.resize( _height );
  for ( auto& line : map )
    line = std::vector<Plib::USTRUCT_MAPINFO>( _width, grass );

  modifyMap( map );

  int header = 0;
  std::ofstream file( _basedir / "map0.mul", std::ofstream::binary | std::ofstream::out );
  for ( int x = 0; x < _width / 8; ++x )
  {
    for ( int y = 0; y < _height / 8; ++y )
    {
      writeFile( file, header );
      for ( int bx = 0; bx < 8; ++bx )
      {
        for ( int by = 0; by < 8; ++by )
        {
          writeFile( file, map[y * 8 + by][x * 8 + bx] );
        }
      }
    }
  }
}

void FileGenerator::modifyMap( std::vector<std::vector<Plib::USTRUCT_MAPINFO>>& map )
{
  Plib::USTRUCT_MAPINFO water{0xa8, -5};
  const int border = 30;
  int y = 0;
  for ( auto& line : map )
  {
    int x = 0;
    std::replace_if(
        line.begin(), line.end(),
        [&]( const auto& ) {
          if ( x < border || x > _width - border || y < border || y > _width - border )
          {
            ++x;
            return true;
          }
          ++x;
          return false;
        },
        water );
    ++y;
  }
}

void FileGenerator::generateStatics()
{
  INFO_PRINT << "Generating statics0.mul\n";
  Plib::USTRUCT_IDX idxempty{0xFFffFFff, 0xFFffFFff, 0xFFffFFff};
  std::ofstream file( _basedir / "statics0.mul", std::ofstream::binary | std::ofstream::out );
  std::ofstream fileidx( _basedir / "staidx0.mul", std::ofstream::binary | std::ofstream::out );

  // init statics: [y][x][...]
  std::vector<std::vector<std::vector<Plib::USTRUCT_STATIC>>> statics;
  statics.resize( _height / 8 );
  for ( auto& line : statics )
  {
    line = std::vector<std::vector<Plib::USTRUCT_STATIC>>( _width / 8,
                                                           std::vector<Plib::USTRUCT_STATIC>() );
  }

  modifyStatics( statics );

  Plib::USTRUCT_IDX idx{0, 0, 0xFFffFFff};
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
  auto addstatic = [&]( int x, int y, s8 z, u16 graphic, u16 hue ) {
    statics[y / 8][x / 8].push_back(
        {graphic, static_cast<s8>( x % 8 ), static_cast<s8>( y % 8 ), z, hue} );
  };

  addstatic( 10, 5, -5, 0x3fff, 0 );
  addstatic( 11, 5, 10, 0x3fff, 0 );
}

template <typename T>
void FileGenerator::modifyMultis( std::vector<std::vector<T>>& multis )
{
  auto elem = [&]( u16 graphic, s16 x, s16 y, s16 z, u32 flags ) {
    T e{};
    e.graphic = graphic;
    e.x = x;
    e.y = y;
    e.z = z;
    e.flags = flags;
    return e;
  };
  multis.resize( 0x18, std::vector<T>() );
  // first boat
  multis[0x0] = std::vector<T>{
      elem( 0x3edd, 0, 0, 0, 0 ),   elem( 0x3e4e, 1, 4, 0, 1 ),   elem( 0x3eae, 0, -4, 0, 1 ),
      elem( 0x3eb2, 2, 0, 0, 1 ),   elem( 0x3eb1, -2, 0, 0, 1 ),  elem( 0x3eac, 0, -2, 0, 0 ),
      elem( 0x3e9e, 1, -3, 0, 0 ),  elem( 0x3ead, 0, -3, 0, 0 ),  elem( 0x3e9d, -1, -3, 0, 0 ),
      elem( 0x3eac, 1, -1, 0, 0 ),  elem( 0x3ea1, -1, -1, 0, 0 ), elem( 0x3ea0, 1, -2, 0, 0 ),
      elem( 0x3e9f, -1, -2, 0, 0 ), elem( 0x3eac, 0, -1, 0, 0 ),  elem( 0x3ea1, -1, 0, 0, 0 ),
      elem( 0x3eac, 1, 0, 0, 0 ),   elem( 0x3e9c, 1, -4, 0, 0 ),  elem( 0x3e9b, -1, -4, 0, 0 ),
      elem( 0x3ee4, 2, -1, 0, 0 ),  elem( 0x3eb1, -2, -1, 0, 0 ), elem( 0x3e9a, 0, -5, 0, 0 ),
      elem( 0x3ede, 2, 0, 0, 0 ),   elem( 0x3edc, -1, 1, 0, 0 ),  elem( 0x3ea1, -1, 1, 0, 0 ),
      elem( 0x3eac, 0, 1, 0, 0 ),   elem( 0x3eac, 1, 1, 0, 0 ),   elem( 0x3ea5, -1, 2, 0, 0 ),
      elem( 0x3eac, 0, 2, 0, 0 ),   elem( 0x3ea6, 1, 2, 0, 0 ),   elem( 0x3eb2, 2, 1, 0, 0 ),
      elem( 0x3eb1, -2, 1, 0, 0 ),  elem( 0x3ece, -2, 2, 0, 0 ),  elem( 0x3ea7, -1, 3, 0, 0 ),
      elem( 0x3eac, 0, 3, 0, 0 ),   elem( 0x3ea8, 1, 3, 0, 0 ),   elem( 0x3eb6, -1, 4, 0, 0 ),
      elem( 0x3eaa, 0, 4, 0, 0 ),   elem( 0x3ebc, 0, 5, 0, 0 )};
  multis[0x1] = std::vector<T>{
      elem( 0x3e5a, 0, 0, 0, 0 ),   elem( 0x3e55, -4, 0, 0, 1 ),  elem( 0x3e65, 4, 0, 0, 1 ),
      elem( 0x3e85, 0, 2, 0, 1 ),   elem( 0x3e8a, 0, -2, 0, 1 ),  elem( 0x3e88, 0, -1, 0, 0 ),
      elem( 0x3e8b, 1, 1, 0, 0 ),   elem( 0x3e8b, -1, 1, 0, 0 ),  elem( 0x3e8b, 0, 1, 0, 0 ),
      elem( 0x3e8b, 1, 0, 0, 0 ),   elem( 0x3e5b, 1, -1, 0, 0 ),  elem( 0x3e88, 1, -1, 0, 0 ),
      elem( 0x3e8b, -1, 0, 0, 0 ),  elem( 0x3e88, -1, -1, 0, 0 ), elem( 0x3e8a, -1, -2, 0, 0 ),
      elem( 0x3e8b, -2, 0, 0, 0 ),  elem( 0x3e8d, -2, -1, 0, 0 ), elem( 0x3e59, 0, 2, 0, 0 ),
      elem( 0x3e3f, -1, 2, 0, 0 ),  elem( 0x3e7e, 2, 1, 0, 0 ),   elem( 0x3e8a, 1, -2, 0, 0 ),
      elem( 0x3e7f, 2, -1, 0, 0 ),  elem( 0x3e8b, 2, 0, 0, 0 ),   elem( 0x3e85, 1, 2, 0, 0 ),
      elem( 0x3e8c, -2, 1, 0, 0 ),  elem( 0x3e8b, 3, 0, 0, 0 ),   elem( 0x3e7d, 3, -1, 0, 0 ),
      elem( 0x3e90, -3, -1, 0, 0 ), elem( 0x3e8f, -3, 0, 0, 0 ),  elem( 0x3e5c, 2, -2, 0, 0 ),
      elem( 0x3e7c, 3, 1, 0, 0 ),   elem( 0x3e8e, -3, 1, 0, 0 ),  elem( 0x3e66, 4, 1, 0, 0 ),
      elem( 0x3e67, 4, -1, 0, 0 ),  elem( 0x3e91, -4, 1, 0, 0 ),  elem( 0x3e94, -4, -1, 0, 0 ),
      elem( 0x3e63, -5, 0, 0, 0 ),  elem( 0x3e69, 5, 0, 0, 0 )};
  multis[0x2] = std::vector<T>{
      elem( 0x3ee2, 0, 0, 0, 0 ),   elem( 0x3e4b, 0, -4, 0, 1 ),  elem( 0x3eb9, 0, 4, 0, 1 ),
      elem( 0x3eb1, -2, 0, 0, 1 ),  elem( 0x3eb2, 2, 0, 0, 1 ),   elem( 0x3eac, 1, 0, 0, 0 ),
      elem( 0x3eac, 1, -1, 0, 0 ),  elem( 0x3eac, 1, 1, 0, 0 ),   elem( 0x3eac, 0, 1, 0, 0 ),
      elem( 0x3eac, 0, -1, 0, 0 ),  elem( 0x3ee1, -1, 1, 0, 0 ),  elem( 0x3ea1, -1, -1, 0, 0 ),
      elem( 0x3ea1, -1, 0, 0, 0 ),  elem( 0x3ea1, -1, 1, 0, 0 ),  elem( 0x3eb1, -2, 1, 0, 0 ),
      elem( 0x3eb1, -2, -1, 0, 0 ), elem( 0x3ea0, 1, -2, 0, 0 ),  elem( 0x3e9f, -1, -2, 0, 0 ),
      elem( 0x3eac, 0, -2, 0, 0 ),  elem( 0x3ee3, 2, 0, 0, 0 ),   elem( 0x3ee6, 2, -1, 0, 0 ),
      elem( 0x3eb2, 2, 1, 0, 0 ),   elem( 0x3ea6, 1, 2, 0, 0 ),   elem( 0x3eac, 0, 2, 0, 0 ),
      elem( 0x3ea5, -1, 2, 0, 0 ),  elem( 0x3ead, 0, -3, 0, 0 ),  elem( 0x3e9d, -1, -3, 0, 0 ),
      elem( 0x3e9e, 1, -3, 0, 0 ),  elem( 0x3ee0, -2, 2, 0, 0 ),  elem( 0x3ea8, 1, 3, 0, 0 ),
      elem( 0x3ea7, -1, 3, 0, 0 ),  elem( 0x3eac, 0, 3, 0, 0 ),   elem( 0x3eb6, -1, 4, 0, 0 ),
      elem( 0x3eb5, 1, 4, 0, 0 ),   elem( 0x3e9b, -1, -4, 0, 0 ), elem( 0x3e9c, 1, -4, 0, 0 ),
      elem( 0x3eb4, 0, 5, 0, 0 ),   elem( 0x3ec4, 0, -5, 0, 0 )};
  multis[0x3] = std::vector<T>{
      elem( 0x3e6c, 0, 0, 0, 0 ),   elem( 0x3e50, 4, 0, 0, 1 ),   elem( 0x3e93, -4, 0, 0, 1 ),
      elem( 0x3e8a, 0, -2, 0, 1 ),  elem( 0x3e85, 0, 2, 0, 1 ),   elem( 0x3e8b, 1, 1, 0, 0 ),
      elem( 0x3e8b, -1, 1, 0, 0 ),  elem( 0x3e8b, -1, 0, 0, 0 ),  elem( 0x3e88, -1, -1, 0, 0 ),
      elem( 0x3e6d, 1, -1, 0, 0 ),  elem( 0x3e8b, 0, 1, 0, 0 ),   elem( 0x3e88, 1, -1, 0, 0 ),
      elem( 0x3e88, 0, -1, 0, 0 ),  elem( 0x3e8b, 1, 0, 0, 0 ),   elem( 0x3e8b, 2, 0, 0, 0 ),
      elem( 0x3e7e, 2, 1, 0, 0 ),   elem( 0x3e85, 1, 2, 0, 0 ),   elem( 0x3e8a, 1, -2, 0, 0 ),
      elem( 0x3e8a, -1, -2, 0, 0 ), elem( 0x3e8c, -2, 1, 0, 0 ),  elem( 0x3e8b, -2, 0, 0, 0 ),
      elem( 0x3e8d, -2, -1, 0, 0 ), elem( 0x3e6b, 0, 2, 0, 0 ),   elem( 0x3e41, -1, 2, 0, 0 ),
      elem( 0x3e7f, 2, -1, 0, 0 ),  elem( 0x3e7d, 3, -1, 0, 0 ),  elem( 0x3e8b, 3, 0, 0, 0 ),
      elem( 0x3e7c, 3, 1, 0, 0 ),   elem( 0x3e6e, 2, -2, 0, 0 ),  elem( 0x3e8e, -3, 1, 0, 0 ),
      elem( 0x3e8f, -3, 0, 0, 0 ),  elem( 0x3e90, -3, -1, 0, 0 ), elem( 0x3e79, 4, 1, 0, 0 ),
      elem( 0x3e7b, 4, -1, 0, 0 ),  elem( 0x3e94, -4, -1, 0, 0 ), elem( 0x3e91, -4, 1, 0, 0 ),
      elem( 0x3e76, 5, 0, 0, 0 ),   elem( 0x3e95, -5, 0, 0, 0 )};
  // end first boat
}

template <typename T>
void FileGenerator::writeMultis( std::vector<std::vector<T>>& multis )
{
  Plib::USTRUCT_IDX idx{0, 0, 0xFFffFFff};
  Plib::USTRUCT_IDX idxempty{0xFFffFFff, 0xFFffFFff, 0xFFffFFff};
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
  INFO_PRINT << "Generating multi.mul\n";
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
}  // namespace PolTool
}  // namespace Pol
