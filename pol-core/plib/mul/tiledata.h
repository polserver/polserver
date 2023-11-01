#pragma once

#include "plib/ustruct.h"

namespace Pol::Plib::MUL
{
struct Tiledata_Legacy
{
  // Bytes to skip in tiledata.mul (0x68800) to access tile blocks
  static constexpr size_t tilesStart = 0x68800;
  static constexpr size_t tileSize = sizeof( USTRUCT_TILE );
  static constexpr size_t blockSize = 4 + 32 * tileSize;  // DWORD + 32 tiles per block
};
static_assert( Tiledata_Legacy::blockSize == 1188, "Size mismatch" );

struct Tiledata_HSA
{
  // Bytes to skip in tiledata.mul (0x78800) to access tile blocks in the new HSA format
  static constexpr size_t tilesStart = 0x78800;
  static constexpr size_t tileSize = sizeof( USTRUCT_TILE_HSA );
  static constexpr size_t blockSize = 4 + 32 * tileSize;  // DWORD + 32 tiles per block
};
static_assert( Tiledata_HSA::blockSize == 1316, "Size mismatch" );


class TiledataInfo
{
  bool _is_hsa = false;
  size_t _max_tile_id = 0;

  template <typename T>
  static bool valid_size( size_t filesize )
  {
    if ( filesize <= T::tilesStart )
      return false;

    size_t remainder = ( filesize - T::tilesStart ) % T::blockSize;
    if ( remainder == 0 )
      return true;

    // the last block might not have all 32 tiles, so we also check if an integer number of
    // them fit here. (That shouldn't be the case for the original files, but we never know what
    // people throw our way)
    return ( remainder - 4 ) % T::tileSize == 0;
  }

  template <typename T>
  static size_t get_max_tile_id( size_t filesize )
  {
    if ( filesize <= T::tilesStart )
      return 0;

    const size_t nblocks = ( filesize - T::tilesStart ) / T::blockSize;
    const size_t remainder = ( filesize - T::tilesStart ) % T::blockSize;
    const size_t leftovers = remainder / T::tileSize;

    return ( 32 * nblocks + leftovers - 1 );
  }

  void detect( size_t filesize )
  {
    if ( valid_size<Tiledata_Legacy>( filesize ) )
    {
      _max_tile_id = get_max_tile_id<Tiledata_Legacy>( filesize );
    }
    else if ( valid_size<Tiledata_HSA>( filesize ) )
    {
      _is_hsa = true;
      _max_tile_id = get_max_tile_id<Tiledata_HSA>( filesize );
    }
    else
    {
      // it's neither - don't try to read it
      _max_tile_id = 0;
    }
  }

public:
  TiledataInfo( size_t filesize ) { detect( filesize ); }
  bool is_hsa() { return _is_hsa; }
  size_t max_tile_id() { return _max_tile_id; };
};

};  // namespace Pol::Plib::MUL