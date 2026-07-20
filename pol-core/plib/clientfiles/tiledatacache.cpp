/** @file
 * In-memory tiledata cache (land + item tiles, verdata-patched):
 * readtile/readlandtile and the tileheight/tile_uoflags_read accessors.
 *
 * @par History
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include <cstdio>
#include <cstring>

#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/stlutil.h"
#include "systemstate.h"
#include "uoclientfiles.h"
#include "uofile.h"
#include "uofilei.h"
#include "ustruct.h"


namespace Pol::Plib
{
#define VERFILE_TILEDATA 0x1E
#define TILEDATA_TILES 0x68800
#define TILEDATA_TILES_HSA 0x78800

unsigned int UoClientFiles::landtile_uoflags_read( unsigned short landtile ) const
{
  passert_always( landtile < LANDTILE_COUNT );
  return landtile_flags_arr[landtile];
}

bool UoClientFiles::check_verdata( unsigned int file, unsigned int block,
                                   const USTRUCT_VERSION*& vrec ) const
{
  return vidx[file].find( block, vrec );
}

bool UoClientFiles::seekto_newer_version( unsigned int file, unsigned int block ) const
{
  const USTRUCT_VERSION* vrec;
  if ( vidx[file].find( block, vrec ) )
  {
    fseek( verfile, vrec->filepos, SEEK_SET );
    return true;
  }

  return false;
}

void UoClientFiles::readtile( unsigned short tilenum, USTRUCT_TILE* tile ) const
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
      fseek( verfile, filepos, SEEK_CUR );
      if ( fread( tile, sizeof *tile, 1, verfile ) != 1 )
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
      fseek( tilefile, filepos, SEEK_SET );
      if ( fread( tile, sizeof *tile, 1, tilefile ) != 1 )
        throw std::runtime_error( "readtile: fread(tile) failed." );
    }
  }

  // ensure string is null-terminated
  tile->name[sizeof( tile->name ) - 1] = '\0';
}

void UoClientFiles::readtile( unsigned short tilenum, USTRUCT_TILE_HSA* tile ) const
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
      fseek( verfile, filepos, SEEK_CUR );
      if ( fread( tile, sizeof *tile, 1, verfile ) != 1 )
        throw std::runtime_error( "readtile: fread(tile) failed." );
    }
    else
    {
      int filepos;
      filepos = TILEDATA_TILES_HSA + ( block * 4 ) +  // skip headers of all previous blocks
                4 +                                   // skip my header
                ( sizeof( USTRUCT_TILE_HSA ) * tilenum );
      fseek( tilefile, filepos, SEEK_SET );
      if ( fread( tile, sizeof *tile, 1, tilefile ) != 1 )
        throw std::runtime_error( "readtile: fread(tile) failed." );
    }
  }

  // ensure string is null-terminated
  tile->name[sizeof( tile->name ) - 1] = '\0';
}


void UoClientFiles::readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE* landtile ) const
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
      fseek( verfile, filepos, SEEK_CUR );
      if ( fread( landtile, sizeof *landtile, 1, verfile ) != 1 )
        throw std::runtime_error( "readlandtile: fread(landtile) failed." );
    }
    else
    {
      int filepos;
      filepos = ( block * 4 ) +  // skip headers of all previous blocks
                4 +              // skip my header
                ( sizeof( USTRUCT_LAND_TILE ) * tilenum );
      fseek( tilefile, filepos, SEEK_SET );
      if ( fread( landtile, sizeof *landtile, 1, tilefile ) != 1 )
        throw std::runtime_error( "readlandtile: fread(landtile) failed." );
    }
  }

  // ensure string is null-terminated
  landtile->name[sizeof( landtile->name ) - 1] = '\0';
}

void UoClientFiles::readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE_HSA* landtile ) const
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
      fseek( verfile, filepos, SEEK_CUR );
      if ( fread( landtile, sizeof *landtile, 1, verfile ) != 1 )
        throw std::runtime_error( "readlandtile: fread(landtile) failed." );
    }
    else
    {
      int filepos;
      filepos = ( block * 4 ) +  // skip headers of all previous blocks
                4 +              // skip my header
                ( sizeof( USTRUCT_LAND_TILE_HSA ) * tilenum );
      fseek( tilefile, filepos, SEEK_SET );
      if ( fread( landtile, sizeof *landtile, 1, tilefile ) != 1 )
        throw std::runtime_error( "readlandtile: fread(landtile) failed." );
    }
  }

  // ensure string is null-terminated
  landtile->name[sizeof( landtile->name ) - 1] = '\0';
}

void UoClientFiles::read_objinfo( u16 graphic, USTRUCT_TILE& objinfo ) const
{
  readtile( graphic, &objinfo );
}

void UoClientFiles::read_objinfo( u16 graphic, USTRUCT_TILE_HSA& objinfo ) const
{
  readtile( graphic, &objinfo );
}


char UoClientFiles::tileheight_read( unsigned short tilenum ) const
{
  u8 height;
  u32 flags;

  if ( tilenum <= systemstate.config.max_tile_id )
  {
    height = tiledata[tilenum].height;
    flags = tiledata[tilenum].flags;
  }
  else
  {
    if ( cfg_use_new_hsa_format )
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

unsigned char UoClientFiles::tilelayer_read( unsigned short tilenum ) const
{
  if ( tilenum <= systemstate.config.max_tile_id )
  {
    return tiledata[tilenum].layer;
  }

  if ( cfg_use_new_hsa_format )
  {
    USTRUCT_TILE_HSA tile;
    tile.layer = 0;
    readtile( tilenum, &tile );
    return tile.layer;
  }

  USTRUCT_TILE tile;
  tile.layer = 0;
  readtile( tilenum, &tile );
  return tile.layer;
}

u16 UoClientFiles::tileweight_read( unsigned short tilenum ) const
{
  if ( cfg_use_new_hsa_format )
  {
    USTRUCT_TILE_HSA tile;
    tile.weight = 1;
    readtile( tilenum, &tile );
    return tile.weight;
  }

  USTRUCT_TILE tile;
  tile.weight = 1;
  readtile( tilenum, &tile );
  return tile.weight;
}

u32 UoClientFiles::tile_uoflags_read( unsigned short tilenum ) const
{
  if ( tilenum <= systemstate.config.max_tile_id )
  {
    return tiledata[tilenum].flags;
  }

  if ( cfg_use_new_hsa_format )
  {
    USTRUCT_TILE_HSA tile;
    tile.flags = 0;
    readtile( tilenum, &tile );
    return tile.flags;
  }

  USTRUCT_TILE tile;
  tile.flags = 0;
  readtile( tilenum, &tile );
  return tile.flags;
}


void UoClientFiles::read_veridx()
{
  int num_version_records;
  USTRUCT_VERSION vrec;

  if ( verfile != nullptr )
  {
    // FIXME: should read this once per run, per file.
    fseek( verfile, 0, SEEK_SET );
    if ( fread( &num_version_records, sizeof num_version_records, 1, verfile ) !=
         1 )  // ENDIAN-BROKEN
      throw std::runtime_error( "read_veridx: fread(num_version_records) failed." );

    for ( int i = 0; i < num_version_records; i++ )
    {
      if ( fread( &vrec, sizeof vrec, 1, verfile ) != 1 )
        throw std::runtime_error( "read_veridx: fread(vrec) failed." );

      if ( vrec.file < vidx_count )
      {
        vidx[vrec.file].insert( vrec );
      }
    }
  }
}

void UoClientFiles::read_tiledata()
{
  tiledata = new TileData[systemstate.config.max_tile_id + 1];

  for ( u32 graphic_i = 0; graphic_i <= systemstate.config.max_tile_id; ++graphic_i )
  {
    u16 graphic = static_cast<u16>( graphic_i );
    if ( cfg_use_new_hsa_format )
    {
      USTRUCT_TILE_HSA objinfo;
      memset( &objinfo, 0, sizeof objinfo );
      readtile( (u16)graphic, &objinfo );

      tiledata[graphic].height = objinfo.height;
      tiledata[graphic].layer = objinfo.layer;
      tiledata[graphic].flags = objinfo.flags;
    }
    else
    {
      USTRUCT_TILE objinfo;
      memset( &objinfo, 0, sizeof objinfo );
      readtile( graphic, &objinfo );

      tiledata[graphic].height = objinfo.height;
      tiledata[graphic].layer = objinfo.layer;
      tiledata[graphic].flags = objinfo.flags;
    }
  }
}

void UoClientFiles::clear_tiledata()
{
  delete[] tiledata;
}

void UoClientFiles::read_landtiledata()
{
  for ( u16 objtype = 0; objtype < LANDTILE_COUNT; ++objtype )
  {
    if ( cfg_use_new_hsa_format )
    {
      USTRUCT_LAND_TILE_HSA landtile;
      readlandtile( objtype, &landtile );

      landtile_flags_arr[objtype] = landtile.flags;
    }
    else
    {
      USTRUCT_LAND_TILE landtile;
      readlandtile( objtype, &landtile );

      landtile_flags_arr[objtype] = landtile.flags;
    }
  }
}
}  // namespace Pol::Plib
