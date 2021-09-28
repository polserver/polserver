#pragma once

#include <stdexcept>

namespace Pol::Plib::MUL
{
struct Map
{
  static constexpr size_t blockWidth = 8;
  static constexpr size_t blockHeight = 8;
  static constexpr size_t cellSize = 3;

  // DWORD + 64 cells (3 bytes) per block
  static constexpr size_t blockSize = 4 + cellSize * blockWidth * blockHeight;

  static constexpr size_t expected_blocks( int width, int height )
  {
    return ( width / blockWidth ) * ( height / blockHeight );
  }

  static constexpr bool valid_size( size_t filesize, int width, int height )
  {
    if ( width % blockWidth != 0 )
      return false;

    if ( height % blockHeight != 0 )
      return false;

    size_t nblocks = filesize / Map::blockSize;
    if ( expected_blocks( width, height ) != nblocks )
      return false;

    return true;
  }
};

class MapInfo
{
  int _width = 0;
  int _height = 0;

  bool _guessed_correctly = false;

  void set_default_size();
  bool guess_size( size_t filesize );

public:
  int width() { return _width; }
  int height() { return _height; }

  bool guessed() { return _guessed_correctly; }
  bool matches_filesize() { return Map::valid_size( filesize, _width, _height ); }
  
  const int mapid = -1;
  const size_t filesize = 0;

  MapInfo( int mapid ) : mapid( mapid )
  {
    set_default_size();
  }

  MapInfo( int mapid, size_t filesize ) : mapid( mapid ), filesize( filesize )
  {
    set_default_size();
    if ( _height == 0 && _width == 0 )
    {
      _guessed_correctly = guess_size( filesize );
    }
  }
};


inline bool MapInfo::guess_size( size_t map_size )
{
  // Guessing is only implemented for britannia now
  if ( mapid != 0 && mapid != 1 )
    return false;

  size_t mapblocks = map_size / Map::blockSize;
  switch ( mapblocks )
  {
  case Map::expected_blocks( 6144, 4096 ):
    _width = 6144;
    _height = 4096;
    break;

  case Map::expected_blocks( 7168, 4096 ):
    _width = 7168;
    _height = 4096;
    break;

  // Last guess. Do we actually need the others?
  default:
    constexpr int common_height = 4096 / Map::blockHeight;

    // Number of blocks has to be divisible by either dimension
    if ( ( mapblocks % common_height ) != 0 )
      return false;

    _height = 4096;
    _width = static_cast<int>( Map::blockWidth * ( mapblocks / common_height ) );
    break;
  }

  return true;
}


inline void MapInfo::set_default_size()
{
  _width = 0;
  _height = 0;

  switch ( mapid )
  {
  case 0:  // britannia / britannia-alt
  case 1:
    // depends on the filesize
    break;
  case 2:  // ilshenar:
    _width = 2304;
    _height = 1600;
    break;
  case 3:  // malas
    _width = 2560;
    _height = 2048;
    break;
  case 4:  // tokuno
    _width = 1448;
    _height = 1448;
    break;
  case 5:  // termur
    _width = 1280;
    _height = 4096;
    break;
  }
}

};  // namespace Pol::Plib::MUL