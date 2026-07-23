/** @file
 * In-memory tiledata cache (land + item tiles, verdata-patched): owns the
 * tiledata.mul/verdata.mul handles, opens + fills the caches, and answers
 * readtile/readlandtile and the tileheight/tile_uoflags accessors.
 *
 * @par History
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/rawtypes.h"
#include "clib/stlutil.h"
#include "plib/mul/tiledata.h"
#include "systemstate.h"
#include "tiledatacache.h"
#include "uoclientfiles.h"
#include "ustruct.h"


namespace Pol::Plib
{
constexpr int VERFILE_TILEDATA = 0x1E;
constexpr int TILEDATA_TILES = 0x68800;
constexpr int TILEDATA_TILES_HSA = 0x78800;

void TileDataCache::open()
{
  // verdata is opened before tiledata so console output matches the historical
  // open_uo_data_files() ordering.
  if ( Clib::FileExists( ( systemstate.config.uo_datafile_root + "verdata.mul" ).c_str() ) )
    verfile_.reset( open_uo_file( "verdata.mul" ) );
  else
    verfile_.reset();

  size_t tiledata_size;
  tilefile_.reset( open_uo_file( "tiledata.mul", &tiledata_size ) );

  // Auto-detect HSA format, find number of blocks, etc
  MUL::TiledataInfo tileinfo( tiledata_size );

  use_new_hsa_format_ = tileinfo.is_hsa();
  systemstate.config.max_tile_id = tileinfo.max_tile_id();

  if ( !systemstate.config.max_tile_id )
  {
    ERROR_PRINTLN(
        "\n"
        "Error reading tiledata.mul:\n"
        " - The file is either corrupted or has an "
        "unknown format.\n" );

    throw std::runtime_error( "Unknown format of tiledata.mul" );
  }

  INFO_PRINTLN(
      "Using auto-detected parameters:\n"
      "\tUseNewHSAFormat = {}\n"
      "\tMaxTileID = {:#x}",
      use_new_hsa_format_, systemstate.config.max_tile_id );
}

void TileDataCache::load()
{
  read_veridx();
  read_tiledata();
  read_landtiledata();
}

void TileDataCache::clear()
{
  tiledata_.clear();
}

unsigned int TileDataCache::landtile_uoflags_read( unsigned short landtile ) const
{
  passert_always( landtile < LANDTILE_COUNT );
  return landtile_flags_arr_[landtile];
}

bool TileDataCache::check_verdata( unsigned int file, unsigned int block,
                                   const USTRUCT_VERSION*& vrec ) const
{
  return vidx_[file].find( block, vrec );
}

bool TileDataCache::seekto_newer_version( unsigned int file, unsigned int block ) const
{
  const USTRUCT_VERSION* vrec;
  if ( vidx_[file].find( block, vrec ) )
  {
    fseek( verfile_.get(), vrec->filepos, SEEK_SET );
    return true;
  }

  return false;
}

void TileDataCache::readtile( unsigned short tilenum, USTRUCT_TILE* tile ) const
{
  memset( tile, 0, sizeof *tile );

  if ( tilenum > systemstate.config.max_tile_id )
  {
    snprintf( tile->name, Clib::arsize( tile->name ), "multi" );
    tile->weight = 0xFF;
  }
  else
  {
    int block;
    block = ( tilenum / 32 );
    if ( seekto_newer_version( VERFILE_TILEDATA, block + 0x200 ) )
    {
      int filepos;
      filepos = 4 + ( sizeof *tile ) * ( tilenum & 0x1F );
      fseek( verfile_.get(), filepos, SEEK_CUR );
      if ( fread( tile, sizeof *tile, 1, verfile_.get() ) != 1 )
        throw std::runtime_error( "readtile: fread(tile) failed." );
    }
    else
    {
      // 512 blocks
      // header 4
      // 32*size (30)
      int filepos;
      filepos = TILEDATA_TILES + ( block * 4 ) +  // skip headers of all previous blocks
                4 +                               // skip my header
                ( sizeof( USTRUCT_TILE ) * tilenum );
      fseek( tilefile_.get(), filepos, SEEK_SET );
      if ( fread( tile, sizeof *tile, 1, tilefile_.get() ) != 1 )
        throw std::runtime_error( "readtile: fread(tile) failed." );
    }
  }

  // ensure string is null-terminated
  tile->name[sizeof( tile->name ) - 1] = '\0';
}

void TileDataCache::readtile( unsigned short tilenum, USTRUCT_TILE_HSA* tile ) const
{
  memset( tile, 0, sizeof *tile );

  if ( tilenum > systemstate.config.max_tile_id )
  {
    snprintf( tile->name, Clib::arsize( tile->name ), "multi" );
    tile->weight = 0xFF;
  }
  else
  {
    int block;
    block = ( tilenum / 32 );
    if ( seekto_newer_version( VERFILE_TILEDATA, block + 0x200 ) )
    {
      int filepos;
      filepos = 4 + ( sizeof *tile ) * ( tilenum & 0x1F );
      fseek( verfile_.get(), filepos, SEEK_CUR );
      if ( fread( tile, sizeof *tile, 1, verfile_.get() ) != 1 )
        throw std::runtime_error( "readtile: fread(tile) failed." );
    }
    else
    {
      int filepos;
      filepos = TILEDATA_TILES_HSA + ( block * 4 ) +  // skip headers of all previous blocks
                4 +                                   // skip my header
                ( sizeof( USTRUCT_TILE_HSA ) * tilenum );
      fseek( tilefile_.get(), filepos, SEEK_SET );
      if ( fread( tile, sizeof *tile, 1, tilefile_.get() ) != 1 )
        throw std::runtime_error( "readtile: fread(tile) failed." );
    }
  }

  // ensure string is null-terminated
  tile->name[sizeof( tile->name ) - 1] = '\0';
}


void TileDataCache::readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE* landtile ) const
{
  memset( landtile, 0, sizeof( *landtile ) );

  if ( tilenum <= MAX_LANDTILE_ID )
  {
    int block;
    block = ( tilenum / 32 );
    if ( seekto_newer_version( VERFILE_TILEDATA, block ) )
    {
      int filepos;
      filepos = 4 + ( sizeof *landtile ) * ( tilenum & 0x1F );
      fseek( verfile_.get(), filepos, SEEK_CUR );
      if ( fread( landtile, sizeof *landtile, 1, verfile_.get() ) != 1 )
        throw std::runtime_error( "readlandtile: fread(landtile) failed." );
    }
    else
    {
      int filepos;
      filepos = ( block * 4 ) +  // skip headers of all previous blocks
                4 +              // skip my header
                ( sizeof( USTRUCT_LAND_TILE ) * tilenum );
      fseek( tilefile_.get(), filepos, SEEK_SET );
      if ( fread( landtile, sizeof *landtile, 1, tilefile_.get() ) != 1 )
        throw std::runtime_error( "readlandtile: fread(landtile) failed." );
    }
  }

  // ensure string is null-terminated
  landtile->name[sizeof( landtile->name ) - 1] = '\0';
}

void TileDataCache::readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE_HSA* landtile ) const
{
  memset( landtile, 0, sizeof( *landtile ) );

  if ( tilenum <= MAX_LANDTILE_ID )
  {
    int block;
    block = ( tilenum / 32 );
    if ( seekto_newer_version( VERFILE_TILEDATA, block ) )
    {
      int filepos;
      filepos = 4 + ( sizeof *landtile ) * ( tilenum & 0x1F );
      fseek( verfile_.get(), filepos, SEEK_CUR );
      if ( fread( landtile, sizeof *landtile, 1, verfile_.get() ) != 1 )
        throw std::runtime_error( "readlandtile: fread(landtile) failed." );
    }
    else
    {
      int filepos;
      filepos = ( block * 4 ) +  // skip headers of all previous blocks
                4 +              // skip my header
                ( sizeof( USTRUCT_LAND_TILE_HSA ) * tilenum );
      fseek( tilefile_.get(), filepos, SEEK_SET );
      if ( fread( landtile, sizeof *landtile, 1, tilefile_.get() ) != 1 )
        throw std::runtime_error( "readlandtile: fread(landtile) failed." );
    }
  }

  // ensure string is null-terminated
  landtile->name[sizeof( landtile->name ) - 1] = '\0';
}

void TileDataCache::read_objinfo( u16 graphic, USTRUCT_TILE& objinfo ) const
{
  readtile( graphic, &objinfo );
}

void TileDataCache::read_objinfo( u16 graphic, USTRUCT_TILE_HSA& objinfo ) const
{
  readtile( graphic, &objinfo );
}


char TileDataCache::tileheight_read( unsigned short tilenum ) const
{
  u8 height;
  u32 flags;

  if ( tilenum <= systemstate.config.max_tile_id )
  {
    height = tiledata_[tilenum].height;
    flags = tiledata_[tilenum].flags;
  }
  else
  {
    if ( use_new_hsa_format_ )
    {
      USTRUCT_TILE_HSA tile;
      readtile( tilenum, &tile );
      height = tile.height;
      flags = tile.flags;
    }
    else
    {
      USTRUCT_TILE tile;
      readtile( tilenum, &tile );
      height = tile.height;
      flags = tile.flags;
    }
  }

  if ( flags & USTRUCT_TILE::FLAG_HALF_HEIGHT )
    return ( height / 2 );
  return height;
}

u32 TileDataCache::tile_uoflags_read( unsigned short tilenum ) const
{
  if ( tilenum <= systemstate.config.max_tile_id )
  {
    return tiledata_[tilenum].flags;
  }

  if ( use_new_hsa_format_ )
  {
    USTRUCT_TILE_HSA tile;
    readtile( tilenum, &tile );
    return tile.flags;
  }

  USTRUCT_TILE tile;
  readtile( tilenum, &tile );
  return tile.flags;
}


void TileDataCache::read_veridx()
{
  int num_version_records;
  USTRUCT_VERSION vrec;

  if ( verfile_.get() != nullptr )
  {
    // FIXME: should read this once per run, per file.
    fseek( verfile_.get(), 0, SEEK_SET );
    if ( fread( &num_version_records, sizeof num_version_records, 1, verfile_.get() ) !=
         1 )  // ENDIAN-BROKEN
      throw std::runtime_error( "read_veridx: fread(num_version_records) failed." );

    for ( int i = 0; i < num_version_records; i++ )
    {
      if ( fread( &vrec, sizeof vrec, 1, verfile_.get() ) != 1 )
        throw std::runtime_error( "read_veridx: fread(vrec) failed." );

      if ( vrec.file < vidx_count )
      {
        vidx_[vrec.file].insert( vrec );
      }
    }
  }
}

void TileDataCache::read_tiledata()
{
  tiledata_.assign( systemstate.config.max_tile_id + 1, TileData{} );

  for ( u32 graphic_i = 0; graphic_i <= systemstate.config.max_tile_id; ++graphic_i )
  {
    u16 graphic = static_cast<u16>( graphic_i );
    if ( use_new_hsa_format_ )
    {
      USTRUCT_TILE_HSA objinfo;
      readtile( graphic, &objinfo );

      tiledata_[graphic].height = objinfo.height;
      tiledata_[graphic].layer = objinfo.layer;
      tiledata_[graphic].flags = objinfo.flags;
    }
    else
    {
      USTRUCT_TILE objinfo;
      readtile( graphic, &objinfo );

      tiledata_[graphic].height = objinfo.height;
      tiledata_[graphic].layer = objinfo.layer;
      tiledata_[graphic].flags = objinfo.flags;
    }
  }
}

void TileDataCache::read_landtiledata()
{
  for ( u16 objtype = 0; objtype < LANDTILE_COUNT; ++objtype )
  {
    if ( use_new_hsa_format_ )
    {
      USTRUCT_LAND_TILE_HSA landtile;
      readlandtile( objtype, &landtile );

      landtile_flags_arr_[objtype] = landtile.flags;
    }
    else
    {
      USTRUCT_LAND_TILE landtile;
      readlandtile( objtype, &landtile );

      landtile_flags_arr_[objtype] = landtile.flags;
    }
  }
}
}  // namespace Pol::Plib
